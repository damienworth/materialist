namespace application {

namespace /* anonymous */ {

void draw_frame(vulkan::context&) noexcept;

}

void
main_loop() noexcept
{
    vk::DynamicLoader dl;
    if (!dl.success()) { ERROR("failed to create dynamic loader"); }
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vulkan::context context;
    vulkan::initialize(context, 800, 600);

    auto& window = context.window;
    assert(*window);

    while (!glfwWindowShouldClose(*window)) {
        glfwPollEvents();
        draw_frame(context);
    }

    context.device->waitIdle();
}

namespace /* anonymous */ {

void
draw_frame(vulkan::context& ctx) noexcept
{
    ctx.device->waitForFences(
        1u,
        &*ctx.inflight_fences[ctx.current_frame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    uint32_t image_index;
    auto     result = ctx.device->acquireNextImageKHR(
        *ctx.swapchain,
        std::numeric_limits<uint64_t>::max(),
        *ctx.image_avail_semaphores[ctx.current_frame],
        vk::Fence(),
        &image_index);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        vulkan::recreate_swapchain(ctx);
        return;
    } else if (
        result != vk::Result::eSuccess &&
        result != vk::Result::eSuboptimalKHR) {
        ERROR("failed to acquire next image");
    }

    if (ctx.images_inflight[image_index]) {
        ctx.device->waitForFences(
            1u,
            &ctx.images_inflight[image_index],
            VK_TRUE,
            std::numeric_limits<uint32_t>::max());
    }
    ctx.images_inflight[image_index] = *ctx.inflight_fences[ctx.current_frame];

    vk::SubmitInfo submit_info;

    vk::Semaphore wait_semaphores[] = {
        *ctx.image_avail_semaphores[ctx.current_frame]};
    vk::PipelineStageFlags wait_stages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &*ctx.command_buffers[image_index];

    vk::Semaphore signal_semaphores[] = {
        *ctx.render_finished_semaphores[ctx.current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    ctx.device->resetFences(1u, &*ctx.inflight_fences[ctx.current_frame]);

    if (ctx.graphics_queue.submit(
            1, &submit_info, *ctx.inflight_fences[ctx.current_frame]) !=
        vk::Result::eSuccess) {
        ERROR("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR present_info;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    vk::SwapchainKHR swapchains[] = {*ctx.swapchain};
    present_info.swapchainCount   = 1;
    present_info.pSwapchains      = swapchains;
    present_info.pImageIndices    = &image_index;

    result = ctx.present_queue.presentKHR(&present_info);
    if (result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR) {
        vulkan::recreate_swapchain(ctx);
    } else if (result != vk::Result::eSuccess) {
        ERROR("failed to present swap chain image!");
    }

    ctx.current_frame = (ctx.current_frame + 1) % vulkan::MAX_FRAMES_IN_FLIGHT;
}

} // namespace

} // namespace application


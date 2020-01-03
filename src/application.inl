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
}

namespace /* anonymous */ {

void
draw_frame(vulkan::context& ctx) noexcept
{
    if (vk::Result::eSuccess !=
        ctx.device->waitForFences(
            *ctx.inflight_fences[ctx.current_frame], true, UINT64_MAX)) {
        ERROR("failed to wait for fence");
    }

    uint32_t image_index;
    ctx.device->acquireNextImageKHR(
        *ctx.swapchain,
        UINT64_MAX,
        *ctx.image_avail_semaphores[ctx.current_frame],
        nullptr,
        &image_index);

    // check if a previous frame is using this image (i.e. there is its
    // fence to wait on)
    if (ctx.images_inflight[image_index]) {
        if (vk::Result::eSuccess !=
            ctx.device->waitForFences(
                ctx.images_inflight[image_index], true, UINT64_MAX)) {
            ERROR("failed to wait for dence");
        }
    }

    // mark the image as now being in use by this frame
    ctx.images_inflight[image_index] = *ctx.inflight_fences[ctx.current_frame];

    vk::Semaphore wait_semaphores[] = {
        *ctx.image_avail_semaphores[ctx.current_frame]};
    vk::PipelineStageFlags wait_stages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::CommandBuffer cmd_buffers[] = {*ctx.command_buffers[image_index]};

    vk::Semaphore signal_semaphores[] = {
        *ctx.render_finished_semaphores[ctx.current_frame]};

    vk::SubmitInfo submit_info(
        1, wait_semaphores, wait_stages, 1, cmd_buffers, 1, signal_semaphores);

    vk::Fence fences[] = {*ctx.inflight_fences[ctx.current_frame]};

    if (vk::Result::eSuccess != ctx.device->resetFences(1, fences)) {
        ERROR("failed to reset fences");
    }

    vk::SubmitInfo submits[] = {submit_info};

    if (vk::Result::eSuccess !=
        ctx.graphics_queue.submit(
            1, submits, *ctx.inflight_fences[ctx.current_frame])) {
        ERROR("failed to submit draw command buffer");
    }

    vk::SwapchainKHR swapchains[] = {*ctx.swapchain};

    vk::PresentInfoKHR present_info(
        1, signal_semaphores, 1, swapchains, &image_index);

    if (vk::Result::eSuccess != ctx.present_queue.presentKHR(present_info)) {
        ERROR("failed to presentKHR");
    }

    ctx.present_queue.waitIdle();
    ++ctx.current_frame;
    if (ctx.current_frame == vulkan::MAX_FRAMES_IN_FLIGHT) {
        ctx.current_frame = 0;
    }
}

} // namespace

} // namespace application


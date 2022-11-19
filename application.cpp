#include "application.h"
#include "vulkanWrapper/image.h"


namespace FF {

	//�ܳ�ʼ��
	void Application::run() {
		initWindow();//��ʼ������
		initVulkan();//��ʼ��Vulkan
		mainLoop();//������ѭ��main
		cleanUp();//������Դ
	}

	void Application::onMouseMove(double xpos, double ypos) {
		mCamera.onMouseMove(xpos, ypos);
	}

	void Application::onKeyDown(CAMERA_MOVE moveDirection) {
		mCamera.move(moveDirection);
	}

	//���ڵĳ�ʼ��
	void Application::initWindow() {
		mWindow = Wrapper::Window::create(mWidth, mHeight);
		mWindow->setApp(shared_from_this());

		mCamera.lookAt(glm::vec3(0.0f, 1.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mCamera.update();

		mCamera.setPerpective(45.0f, (float)mWidth / (float)mHeight, 0.1f, 100.0f);

		mCamera.setSpeed(0.05f);
	}

	//��ʼ��Vulkan
	//1 rendePass ����pipeline 2 ����FrameBuffer
	void Application::initVulkan() {
		mInstance = Wrapper::Instance::create(true);
		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

		mDevice = Wrapper::Device::create(mInstance, mSurface);

		mCommandPool = Wrapper::CommandPool::create(mDevice);

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		

		//descriptor ===========================
		mUniformManager = UniformManager::create();
		mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		//����ģ��
		mModel = Model::create(mDevice);
		mModel->loadModel("../../../assets/kokoro.obj", mDevice);
		//mModel->setModelMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();

		mCommandBuffers.resize(mSwapChain->getImageCount());
		
		createCommandBuffers();

		createSyncObjects();
	}

	void Application::createPipeline() {
		//�����ӿ�
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)mHeight;
		viewport.width = (float)mWidth;
		viewport.height = -(float)mHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = { mWidth, mHeight};

		mPipeline->setViewports({ viewport });
		mPipeline->setScissors({scissor});


		//����shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};

		auto shaderVertex = Wrapper::Shader::create(mDevice, "../../../shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(shaderVertex);

		auto shaderFragment = Wrapper::Shader::create(mDevice, "../../../shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderFragment);
		
		mPipeline->setShaderGroup(shaderGroup);

		//������Ų�ģʽ
		auto vertexBindingDes = mModel->getVertexInputBindingDescriptions();
		auto attributeDes = mModel->getAttributeDescriptions();

		mPipeline->mVertexInputState.vertexBindingDescriptionCount = vertexBindingDes.size();
		mPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = attributeDes.size();
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = attributeDes.data();

		//ͼԪװ��
		mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		//��դ������
		mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;//����ģʽ��Ҫ����gpu����
		mPipeline->mRasterState.lineWidth = 1.0f;//����1��Ҫ����gpu����
		mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		mPipeline->mRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		mPipeline->mRasterState.depthBiasEnable = VK_FALSE;
		mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		mPipeline->mRasterState.depthBiasClamp = 0.0f;
		mPipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		//���ز���
		mPipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		mPipeline->mSampleState.rasterizationSamples = mDevice->getMaxUsableSampleCount();
		mPipeline->mSampleState.minSampleShading = 1.0f;
		mPipeline->mSampleState.pSampleMask = nullptr;
		mPipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
		mPipeline->mSampleState.alphaToOneEnable = VK_FALSE;

		//�����ģ�����
		mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		//��ɫ���

		//�������ɫ������룬�õ��Ļ�Ͻ��������ͨ�����������AND���������
		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		mPipeline->pushBlendAttachment(blendAttachment);

		//1 blend�����ּ��㷽ʽ����һ����������������alphaΪ�����ļ��㣬�ڶ��ֽ���λ����
		//2 ���������logicOp����ô�Ϸ����õ�alphaΪ���������㣬ʧ��
		//3 ColorWrite���룬��Ȼ��Ч�����㿪����logicOP
		//4 ��Ϊ�����ǿ��ܻ��ж��FrameBuffer��������Կ�����Ҫ���BlendAttachment
		mPipeline->mBlendState.logicOpEnable = VK_FALSE;
		mPipeline->mBlendState.logicOp = VK_LOGIC_OP_COPY;

		//���blendAttachment��factor��operation
		mPipeline->mBlendState.blendConstants[0] = 0.0f;
		mPipeline->mBlendState.blendConstants[1] = 0.0f;
		mPipeline->mBlendState.blendConstants[2] = 0.0f;
		mPipeline->mBlendState.blendConstants[3] = 0.0f;

		//uniform�Ĵ���
		mPipeline->mLayoutState.setLayoutCount = 1;

		auto layout = mUniformManager->getDescriptorLayout()->getLayout();
		mPipeline->mLayoutState.pSetLayouts = &layout;
		mPipeline->mLayoutState.pushConstantRangeCount = 0;
		mPipeline->mLayoutState.pPushConstantRanges = nullptr;

		mPipeline->build();
	}

	void Application::createRenderPass() {
		//0���������ͼƬ 1��ResolveͼƬ��MutiSample�� 2��DepthͼƬ

		//0��λ����SwapChainԭ������ͼƬ����Resolve��Ŀ��㣬����Ҫ���õ�SubPass��Resolve����
		VkAttachmentDescription finalAttachmentDes{};
		finalAttachmentDes.format = mSwapChain->getFormat();
		finalAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		finalAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		finalAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		finalAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		finalAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		finalAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		finalAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		mRenderPass->addAttachment(finalAttachmentDes);

		//1��λ����Resolve��ͼƬ�������ز�����ԴͷͼƬ��Ҳ����ɫ�����Ŀ��ͼƬ
		VkAttachmentDescription MutiAttachmentDes{};
		MutiAttachmentDes.format = mSwapChain->getFormat();
		MutiAttachmentDes.samples = mDevice->getMaxUsableSampleCount();
		MutiAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		MutiAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		MutiAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		MutiAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		MutiAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		MutiAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		mRenderPass->addAttachment(MutiAttachmentDes);


		//3��λ����Ȼ���attachment
		VkAttachmentDescription depthAttachmentDes{};
		depthAttachmentDes.format = Wrapper::Image::findDepthFormat(mDevice);
		depthAttachmentDes.samples = mDevice->getMaxUsableSampleCount();
		depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		mRenderPass->addAttachment(depthAttachmentDes);

		//���ڻ��������������Լ���ʽҪ��
		VkAttachmentReference finalAttachmentRef{};
		finalAttachmentRef.attachment = 0;
		finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference mutiAttachmentRef{};
		mutiAttachmentRef.attachment = 1;
		mutiAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//����������
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(mutiAttachmentRef);
		subPass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subPass.setResolveAttachmentReference(finalAttachmentRef);

		subPass.buildSubPassDescription();

		mRenderPass->addSubPass(subPass);

		//������֮���������ϵ
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		mRenderPass->addDependency(dependency);

		mRenderPass->buildRenderPass();
	}

	void Application::createCommandBuffers() {
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);

			mCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = mRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mSwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0, 0 };
			renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

			//0��final   1��muti   2��depth
			std::vector< VkClearValue> clearColors{};
			VkClearValue finalClearColor{};
			finalClearColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			clearColors.push_back(finalClearColor);

			VkClearValue mutiClearColor{};
			mutiClearColor.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			clearColors.push_back(mutiClearColor);

			VkClearValue depthClearColor{};
			depthClearColor.depthStencil = {1.0f, 0};
			clearColors.push_back(depthClearColor);

			renderBeginInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
			renderBeginInfo.pClearValues = clearColors.data();


			mCommandBuffers[i]->beginRenderPass(renderBeginInfo);

			mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getPipeline());

			mCommandBuffers[i]->bindDescriptorSet(mPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));

			//mCommandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffer()->getBuffer() });

			mCommandBuffers[i]->bindVertexBuffer(mModel->getVertexBuffers());

			mCommandBuffers[i]->bindIndexBuffer(mModel->getIndexBuffer()->getBuffer());

			mCommandBuffers[i]->drawIndex(mModel->getIndexCount());

			mCommandBuffers[i]->endRenderPass();

			mCommandBuffers[i]->end();
		}
	}

	void Application::createSyncObjects() {
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
			mImageAvailableSemaphores.push_back(imageSemaphore);

			auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
			mRenderFinishedSemaphores.push_back(renderSemaphore);

			auto fence = Wrapper::Fence::create(mDevice);
			mFences.push_back(fence);
		}
	}

	void Application::recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(mDevice->getDevice());

		cleanupSwapChain();

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface, mCommandPool);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();

		mCommandBuffers.resize(mSwapChain->getImageCount());

		createCommandBuffers();

		createSyncObjects();
	}

	void Application::cleanupSwapChain() {
		mSwapChain.reset();
		mCommandBuffers.clear();
		mPipeline.reset();
		mRenderPass.reset();
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();
	}

	//��ѭ��main
	void Application::mainLoop() {
		while (!mWindow->shouldClose()) {//���ڱ��رս���ѭ��
			mWindow->pollEvents();
			mWindow->processEvent();

			//mModel->update();

			mVPMatrices.mViewMatrix = mCamera.getViewMatrix();
			mVPMatrices.mProjectionMatrix = mCamera.getProjectMatrix();

			mUniformManager->update(mVPMatrices, mModel->getUniform(), mCurrentFrame);

			render();
		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}

	void Application::render() {
		//�ȴ���ǰҪ�ύ��CommandBufferִ�����
		mFences[mCurrentFrame]->block();

		//��ȡ���������е���һ֡
		uint32_t imageIndex{ 0 };
		VkResult result = vkAcquireNextImageKHR(
			mDevice->getDevice(),
			mSwapChain->getSwapChain(),
			UINT64_MAX,
			mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		//���巢���˳ߴ�仯
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}//VK_SUBOPTIMAL_KHR�õ���һ����Ϊ���õ�ͼ�񣬵��Ǳ����ʽ��һ��ƥ��
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Error: failed to acquire next image");
		}

		//�����ύ��Ϣ
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		

		//ͬ����Ϣ����Ⱦ������ʾͼ�����������ʾ��Ϻ󣬲��������ɫ
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		//ָ���ύ��Щ����
		auto commandBuffer = mCommandBuffers[imageIndex]->getCommandBuffer();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore()};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		mFences[mCurrentFrame]->resetFence();
		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence()) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to submit renderCommand");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = {mSwapChain->getSwapChain()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);//��ʼ��Ⱦ

		//������������һ����׼���������ǻ���Ҫ���Լ��ı�־λ�ж�
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow->mWindowResized) {
			recreateSwapChain();
			mWindow->mWindowResized = false;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to present");
		}

		mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
	}


	//������Դ
	void Application::cleanUp() {

		mPipeline.reset();

		mRenderPass.reset();

		mSwapChain.reset();

		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();
	}
}
#include "pipeline.h"

namespace FF::Wrapper {

	Pipeline::Pipeline(const Device::Ptr& device, const RenderPass::Ptr& renderPass) {
		mDevice = device;
		mRenderPass = renderPass;

		mVertexInputState.sType  =VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mLayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	}

	Pipeline::~Pipeline() {
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
		}

		if (mPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
		}
	}

	void Pipeline::setShaderGroup(const std::vector<Shader::Ptr>& shaderGroup) {
		mShaders = shaderGroup;
	}

	void Pipeline::build() {
		//����shader
		std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
		for (const auto& shader : mShaders) {
			VkPipelineShaderStageCreateInfo shaderCreateInfo{};
			shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderCreateInfo.stage = shader->getShaderStage();
			shaderCreateInfo.pName = shader->getShaderEntryPoint().c_str();
			shaderCreateInfo.module = shader->getShaderModule();

			shaderCreateInfos.push_back(shaderCreateInfo);
		}

		//�����ӿڼ���
		mViewportState.viewportCount = static_cast<uint32_t>(mViewports.size());
		mViewportState.pViewports = mViewports.data();
		mViewportState.scissorCount = static_cast<uint32_t>(mScissors.size());
		mViewportState.pScissors = mScissors.data();

		//blending
		mBlendState.attachmentCount = static_cast<uint32_t>(mBlendAttachmentStates.size());
		mBlendState.pAttachments = mBlendAttachmentStates.data();

		//layout ����
		if (mLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(mDevice->getDevice(), mLayout, nullptr);
		}

		if (vkCreatePipelineLayout(mDevice->getDevice(), &mLayoutState, nullptr, &mLayout) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create pipeline layout");
		}

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		//����shader
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderCreateInfos.size());
		pipelineCreateInfo.pStages = shaderCreateInfos.data();

		pipelineCreateInfo.pVertexInputState = &mVertexInputState;//������Ų�ģʽ
		pipelineCreateInfo.pInputAssemblyState = &mAssemblyState;//ͼԪװ��
		pipelineCreateInfo.pViewportState = &mViewportState;//�����ӿڼ���
		pipelineCreateInfo.pRasterizationState = &mRasterState;//��դ������
		pipelineCreateInfo.pMultisampleState = &mSampleState;//���ز���
		pipelineCreateInfo.pDepthStencilState = &mDepthStencilState;//�����ģ�����	
		pipelineCreateInfo.pColorBlendState = &mBlendState;
		pipelineCreateInfo.layout = mLayout;
		pipelineCreateInfo.renderPass = mRenderPass->getRenderPass(); 
		pipelineCreateInfo.subpass = 0;

		//�Դ��ڵ�pipelineΪ�������д���������죬������Ҫָ��flagsΪVK_PIPELINE_CREATE_DERIVATIVE_BIT
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;

		if (mPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(mDevice->getDevice(), mPipeline, nullptr);
		}

		//pipeline cache�����Խ�������ݴ��뻺�棬�ڶ��pipeline����ʹ��,Ҳ���Դ浽�ļ�����ͬ�������
		if (vkCreateGraphicsPipelines(mDevice->getDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create pipeline");
		}
	}
}
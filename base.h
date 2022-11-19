#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <set>
#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN		//����GLFW��֧��
#include <glfw3.h>
#include <vulkan/vulkan.h>


#include "FilePath.h"//�����ļ���·��

//�����Ĳ���ģʽ
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"//��������
};


struct VPMatrices {
	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;

	VPMatrices() {
		mViewMatrix = glm::mat4(1.0f);
		mProjectionMatrix = glm::mat4(1.0f);
	}
};

struct ObjectUniform {
	glm::mat4 mModelMatrix;

	ObjectUniform() {
		mModelMatrix = glm::mat4(1.0f);
	}
};
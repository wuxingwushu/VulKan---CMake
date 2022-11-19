#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace FF {

	void Model::loadModel(const std::string& path, const Wrapper::Device::Ptr& device) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string warn;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
			throw std::runtime_error("Error: failed to load model");
		}


		//printf("%d\n",int(shapes.size()));//OBJģ��,�ķּ�����
		for (const auto& shape : shapes) {
			//printf(shape.name.c_str());�ּ����֣����Ի�����ͼ����
			for (const auto& index : shape.mesh.indices) {
				//��ȡ������λ������
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 2]);
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 0]);
				mPositions.push_back(attrib.vertices[3 * index.vertex_index + 1]);

				//ȡ��uvֵ
				mUVs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
				mUVs.push_back(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);

				//����
				mIndexDatas.push_back(mIndexDatas.size());
			}
		}

		mPositionBuffer = Wrapper::Buffer::createVertexBuffer(device, mPositions.size() * sizeof(float), mPositions.data());

		mUVBuffer = Wrapper::Buffer::createVertexBuffer(device, mUVs.size() * sizeof(float), mUVs.data());

		mIndexBuffer = Wrapper::Buffer::createIndexBuffer(device, mIndexDatas.size() * sizeof(float), mIndexDatas.data());
	}
}
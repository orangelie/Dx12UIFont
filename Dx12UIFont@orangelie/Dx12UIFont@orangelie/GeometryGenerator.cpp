#include "GeometryGenerator.h"

namespace orangelie
{
	CLASSIFICATION_C1(GeometryGenerator);

	GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float width, float height, float depth)
	{
		GeometryGenerator::MeshData meshData;

		meshData.Vertices.resize(4);
		meshData.Indices.resize(6);

		meshData.Vertices[0] = Vertex(
			x, y - height, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f);

		meshData.Vertices[1] = Vertex(
			x, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 0.0f);

		meshData.Vertices[2] = Vertex(
			x + width, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f);

		meshData.Vertices[3] = Vertex(
			x + width, y - height, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f);

		meshData.Indices[0] = 0;
		meshData.Indices[1] = 1;
		meshData.Indices[2] = 2;

		meshData.Indices[3] = 0;
		meshData.Indices[4] = 2;
		meshData.Indices[5] = 3;

		return meshData;
	}
}
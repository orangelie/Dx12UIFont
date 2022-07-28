#pragma once

#include "Utils.h"

namespace orangelie
{
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;

	class GeometryGenerator
	{
	public:
		CLASSIFICATION_H(GeometryGenerator);

		struct Vertex
		{
			Vertex(
				DirectX::XMFLOAT3 Position,
				DirectX::XMFLOAT3 Normal,
				DirectX::XMFLOAT3 Tangent,
				DirectX::XMFLOAT2 TexCoord)
			{
				this->Position = Position;
				this->Normal = Normal;
				this->Tangent = Tangent;
				this->TexCoord = TexCoord;
			}

			Vertex(
				float PositionX, float PositionY, float PositionZ,
				float NormalX, float NormalY, float NormalZ,
				float TangentX, float TangentY, float TangentZ,
				float TexCoordX, float TexCoordY)
			{
				this->Position = DirectX::XMFLOAT3(PositionX, PositionY, PositionZ);
				this->Normal = DirectX::XMFLOAT3(NormalX, NormalY, NormalZ);
				this->Tangent = DirectX::XMFLOAT3(TangentX, TangentY, TangentZ);
				this->TexCoord = DirectX::XMFLOAT2(TexCoordX, TexCoordY);
			}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT2 TexCoord;
		};

		struct MeshData
		{
			std::vector<u16> Indices16() const
			{
				std::vector<u16> indices16(Indieces.size());

				for (size_t i = 0; i < Indieces.size(); ++i)
				{
					indices16[i] = static_cast<u16>(Indieces[i]);
				}

				return indices16;
			}

			std::vector<Vertex> Vertices;
			std::vector<u32> Indieces;
		};

		MeshData CreateQuad(float x, float y, float width, float height, float depth);

	private:


	};
}
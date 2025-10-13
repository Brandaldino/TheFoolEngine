#include "tfpch.h"
#include "MeshData.h"

namespace TheFoolEngine
{
	MeshData::MeshData(const std::vector<MeshVertexData>& vertices, const std::vector<uint32_t>& indices)
		:m_Vertices(vertices), m_Indices(indices)
	{
	}

	MeshData::~MeshData()
	{
	}

	void MeshData::AddTexture(const TextureType& type, const Ref<Texture2D>& texture)
	{
		m_Textures[type].push_back(texture);
	}

	std::vector<Ref<Texture2D>> MeshData::GetTexture(const TextureType& type) const
	{
		auto it = m_Textures.find(type);
		return it != m_Textures.end() ? it->second : std::vector<Ref<Texture2D>>();
	}

	void MeshData::IntegrateTextures()
	{
		for (auto& its : m_Textures)
		{
			auto subtextureinfo = CreateRef<SubTextureInfo>();
			subtextureinfo->Type = its.first;

			// analyze average size
			auto scalelevel = AnalyzeScaleLevel(its.second);

			// Determine the number of columns and rows. 
			// Strategy: Prioritize ensuring that the number of columns is greater than or equal to the number of rows.
			unsigned int column = its.second.size();
			unsigned int row = 1;
			CalcRowAndColumn(row, column);

			subtextureinfo->AtlasSize.x = scalelevel.x * (float)column;
			subtextureinfo->AtlasSize.y = scalelevel.y * (float)row;

			// init res texture
			Ref<Texture2D> resTexture = Texture2D::Create(scalelevel.x * (float)column, scalelevel.y * (float)row);

			glm::vec2 offset = glm::vec2(0.0f);
			glm::vec2 beginPos = scalelevel / 2.0f;

			unsigned int columnCount = 1;
			unsigned int rowCount = 1;

			for (auto& it : its.second)
			{
				// create sub-texture ptr 
				SubTexture* temp = nullptr;

				// set subtexture info
				temp->Texture = it;
				temp->Size.x = it->GetWidth();
				temp->Size.y = it->GetHeight();

				temp->UVScale.x = scalelevel.x / temp->Size.x;
				temp->UVScale.y = scalelevel.y / temp->Size.y;

				temp->UVOffset = offset;
				temp->Position = beginPos + offset;

				// calc offset
				if (columnCount < column)
				{
					offset.x = scalelevel.x * (float)columnCount;
					offset.y = scalelevel.y * (float)rowCount;
					columnCount++;
				}
				else
				{
					columnCount = 1;
					offset.x = scalelevel.x * (float)columnCount;
					offset.y = scalelevel.y * (float)rowCount;
					rowCount++;
				}

				// sub to resTexture
				resTexture->SetSubTextureData(temp->Texture->GetData(), temp->UVOffset.x, temp->UVOffset.y, temp->Size.x, temp->Size.y, sizeof(SubTexture));
				delete temp;
			}

			subtextureinfo->AtlasTexture = resTexture;
			m_SubTextures[subtextureinfo->Type] = subtextureinfo;
		}
	}

	glm::vec2 MeshData::AnalyzeScaleLevel(const std::vector<Ref<Texture2D>>& data)
	{
		glm::vec2 res = glm::vec2(1.0f);
		for (auto it : data)
		{
			res.x += it->GetWidth();
			res.y += it->GetHeight();
		}
		res /= (float)data.size();
		return res;
	}

	void MeshData::CalcRowAndColumn(unsigned int& row, unsigned int& column)
	{
		unsigned int res = row * column;
		row = std::sqrt(res);
		column = row;

		while (row * column < res)
		{
			if (row < column)
				row++;
			else if (row == column)
				column++;
		}
	}

}



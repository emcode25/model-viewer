#include "model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <SDL3/SDL_log.h>

#include "util.hpp"

Model::Model()
{}

void Model::init(char* path, bool gamma)
{
	gammaCorrection = gamma;
	loadModel(path);
}

void Model::draw(GLuint program)
{
	for(auto mesh : meshes)
	{
		mesh.draw(program);
	}
}

void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Assimp] Could not load the model: %s", importer.GetErrorString());
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for(unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for(unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;

		vertex.position = glm::vec3(mesh->mVertices[i].x,
									mesh->mVertices[i].y,
									mesh->mVertices[i].z);

		vertex.normal = glm::vec3(mesh->mNormals[i].x,
								  mesh->mNormals[i].y,
								  mesh->mNormals[i].z);

		if(mesh->mTextureCoords[0])
		{
			vertex.tex = glm::vec2(mesh->mTextureCoords[0][i].x,
								   mesh->mTextureCoords[0][i].y);
		}
		else
		{
			vertex.tex = glm::vec2(0.0f, 0.0f);
		}

		if(mesh->HasTangentsAndBitangents())
		{
			vertex.tangent = glm::vec3(mesh->mTangents[i].x, 
									   mesh->mTangents[i].y, 
									   mesh->mTangents[i].z);

			vertex.bitangent = glm::vec3(mesh->mBitangents[i].x,
										 mesh->mBitangents[i].y,
										 mesh->mBitangents[i].z);
		}

		vertices.push_back(vertex);
	}

	for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];

		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if(mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, SPECULAR);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, NORMAL);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TexType texType)
{
	std::vector<Texture> textures;

	for(unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		
		bool skip = false;
		for(unsigned int j = 0; j < textures_loaded.size(); ++j)
		{
			if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if(!skip)
		{
			Texture texture;
			texture.id = textureFromFile(str.C_Str(), this->directory, gammaCorrection);
			texture.type = texType;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}

	return textures;
}
#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>
#include <string>

#include <glad/glad.h>

#include <assimp/scene.h>

#include "mesh.hpp"

class Model 
{
	public:
		Model();
		void init(char* path, bool gamma = false);
		void draw(GLuint program);
		
	private:
		std::vector<Texture> textures_loaded;
		std::vector<Mesh> meshes;
		std::string directory;
		bool gammaCorrection;

		void loadModel(std::string path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TexType texType);
};

#endif
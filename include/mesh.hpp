#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

enum TexType
{
	DIFFUSE,
	SPECULAR,
	NORMAL
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Texture
{
	unsigned int id;
	TexType type;
	std::string path;
};

class Mesh
{
	public:
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
		void draw(GLuint program);
		void destroy();

	private:
		void setupMesh();

		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;
		unsigned int VAO, VBO, EBO;
};

#endif
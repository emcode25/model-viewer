#include "util.hpp"

#include <iostream>

#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

std::string readFileToString(std::string filename)
{
	FILE* file = fopen(filename.c_str(), "r");
	std::string contents;

	if(file != NULL)
	{
		fseek(file, 0, SEEK_END);
		long len = ftell(file);
		rewind(file);

		contents.resize(len);
		fread(&contents[0], sizeof(char), len, file);
		fclose(file);

		return contents;
	}

	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[UTIL] File \"%s\" could not be opened.", filename.c_str());

	return std::string("Could not read file.");
}

GLuint textureFromFile(const char* path, const std::string& directory, bool gamma)
{
	SDL_Surface* sdlTex;
	std::string filename = path;
	filename = directory + '/' + filename;

	GLuint textureID;
	glGenTextures(1, &textureID);

	sdlTex = IMG_Load(filename.c_str());

	if(sdlTex && sdlTex->pixels)
	{
		GLenum format;

		switch(sdlTex->format)
		{
			case SDL_PIXELFORMAT_INDEX8:
				format = GL_RED;
				break;
			case SDL_PIXELFORMAT_RGB24:
				format = GL_RGB;
				break;
			case SDL_PIXELFORMAT_RGBA32:
				format = GL_RGBA;
				break;
			default:
				format = GL_RED;
				break;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, sdlTex->w, sdlTex->h, 0, format, GL_UNSIGNED_BYTE, sdlTex->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[UTIL] Texture failed to load at path: %s", path);
	}

	SDL_DestroySurface(sdlTex);
	return textureID;
}

GLuint compileShaderProgram(const char* vertexFilename, const char* fragmentFilename)
{
	const char* vertSource, *fragSource;
	std::string vertStr, fragStr;
	vertStr = readFileToString(vertexFilename);
	fragStr = readFileToString(fragmentFilename);

	vertSource = vertStr.c_str();
	fragSource = fragStr.c_str();

	GLuint vertShader, fragShader;
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertShader, 1, &vertSource, NULL);
	glCompileShader(vertShader);
	checkCompileErrors(vertShader, "VERTEX");

	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);
	checkCompileErrors(fragShader, "FRAGMENT");

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	checkCompileErrors(program, "PROGRAM");

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}

void _CheckGLError(const char* file, int line)
{
	GLenum err(glGetError());

	while(err != GL_NO_ERROR)
	{
		std::string error;
		switch(err)
		{
			case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
			default: error = "UNKNOWN";
		}
		std::cout << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
		err = glGetError();
	}

	return;
}

void checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if(type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if(!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
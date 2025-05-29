#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>
#include <string>

std::string readFileToString(std::string filename);
GLuint textureFromFile(const char* path, const std::string& directory, bool gamma);
GLuint compileShaderProgram(const char* vertexFilename, const char* fragmentFilename);
void checkCompileErrors(GLuint shader, std::string type);
void _CheckGLError(const char* file, int line);

#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

#endif
#include <glad/glad.h>

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "util.hpp"
#include "model.hpp"

constexpr glm::vec3 X_AXIS(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 Y_AXIS(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 Z_AXIS(0.0f, 0.0f, 1.0f);

static SDL_Window* window = NULL;
static SDL_GLContext ctx = {};
static GLuint program = 0;
static Model m;

static int screenWidth = 800.0f;
static int screenHeight = 600.0f;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    //Make info visible for all categories
    SDL_SetHint(SDL_HINT_LOGGING, "app=info,assert=warn,test=verbose,*=info");

    //Set OpenGL 4.6
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    //Create window and context
    window = SDL_CreateWindow("Model Viewer", screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    ctx = SDL_GL_CreateContext(window);

    //Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[INIT] GLAD failed to initialize.");
        return SDL_APP_FAILURE;
    }
    else
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[INIT] GLAD initialized successfully.");
    }

    //Set OpenGL options
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

	//Compile shaders
	program = compileShaderProgram("../../../../src/shaders/vertex.glsl", "../../../../src/shaders/fragment.glsl");

	//Create model
	char path[1024] = "../../../../data/models/backpack/backpack.obj";
	m.init(path);

    //Start Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    //Set up backend links
    ImGui_ImplSDL3_InitForOpenGL(window, ctx);
    ImGui_ImplOpenGL3_Init();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[INIT] Model Viewer initialized successfully.");
    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    //Let Dear ImGui see what happened too
    ImGui_ImplSDL3_ProcessEvent(event);

    //Look through keypresses
    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.key)
        {
        case SDLK_ESCAPE:
            return SDL_APP_SUCCESS;
        default:
            break;
        }
    }

	if(event->type == SDL_EVENT_WINDOW_RESIZED)
	{
		screenWidth = event->window.data1;
		screenHeight = event->window.data2;
		glViewport(0, 0, event->window.data1, event->window.data2);
	}

    //Check if the user asked to quit
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
	static glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);
	static glm::vec3 modelRot = glm::vec3(0.0f, 0.0f, 0.0f);
	static glm::vec3 camVec = glm::vec3(0.0f, 0.0f, -5.0f);

	static glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
	static glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	static bool enableLighting = true;
	static bool enableNormalMapping = true;
	static bool recompileShaders = false;
	static bool resetLight = false;

	if(recompileShaders)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "[OPENGL] Recompiling shaders...");
		GLuint newProgram = compileShaderProgram("../../../../src/shaders/vertex.glsl", "../../../../src/shaders/fragment.glsl");

		if(newProgram != 0)
		{
			glDeleteProgram(program);
			program = newProgram;
			SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "[OPENGL] Successfully recompiled shaders.");
		}
		else
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "[OPENGL] Shader recompilation failed.");
		}
	}

	if(resetLight)
	{
		lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
		lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		resetLight = false;
	}

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 R = glm::mat4(1.0f);
	glm::mat4 S = glm::mat4(1.0f);

	T = glm::translate(T, modelPos);
	R = glm::toMat4(glm::quat(glm::radians(modelRot)));

	glm::mat4 model = T * R * S;
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, camVec);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), static_cast<float>(screenWidth) / screenHeight, 0.1f, 100.0f);

	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "proj"), 1, GL_FALSE, &proj[0][0]);
	glUniform3fv(glGetUniformLocation(program, "lightColor"), 1, &lightColor[0]);
	glUniform3fv(glGetUniformLocation(program, "lightPosition"), 1, &lightPos[0]);
	glUniform1i(glGetUniformLocation(program, "enableLighting"), enableLighting);
	glUniform1i(glGetUniformLocation(program, "enableNormalMapping"), enableNormalMapping);

	m.draw(program);

	if(ImGui::Begin("Controls"))
	{
		ImGui::SeparatorText("Model");
		ImGui::DragFloat3("Position##mpos", &modelPos[0], 0.25f);
		ImGui::DragFloat3("Rotation##mrot", &modelRot[0], 0.25f);

		ImGui::SeparatorText("Camera");
		ImGui::DragFloat3("Position##cpos", &camVec[0], 0.25f);

		ImGui::SeparatorText("Light");
		ImGui::Checkbox("Enable Lighting", &enableLighting);
		ImGui::Checkbox("Enable Normal Mapping", &enableNormalMapping);
		ImGui::DragFloat3("Position##lpos", &lightPos[0], 0.25f);
		ImGui::ColorEdit3("Color##lcolor", &lightColor[0]);
		resetLight = ImGui::Button("Reset##rlight");

		ImGui::SeparatorText("Shaders");
		recompileShaders = ImGui::Button("Recompile Shaders");
	}
	ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	glDeleteProgram(program);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(window);
}
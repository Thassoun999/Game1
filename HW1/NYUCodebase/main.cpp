#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"

SDL_Window* displayWindow;

//All of the Class 5 material
GLuint LoadTexture(const char *filepath) {
	//Class 5 setup
	//Images
	int w, h, comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image!!\n";
		assert(false);
	}

	//Textures
	GLuint retTexture;
	glGenTextures(1, &retTexture);

	glBindTexture(GL_TEXTURE_2D, retTexture);     
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);     
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);     
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);     
	stbi_image_free(image);     
	return retTexture;


}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    SDL_Event event;
    bool done = false;

	// Setup Before the Loop
	glViewport(0, 0, 640, 360);          
	ShaderProgram program;
	ShaderProgram program2;

	//Texture 
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	//Untextured
	program2.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	//Blend Here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Call Texture Stuff Here
	GLuint starTexture = LoadTexture(RESOURCE_FOLDER "starman.png");
	GLuint watermeloneTexture = LoadTexture(RESOURCE_FOLDER "watermelone.png");

	//Starman
	glm::mat4 projectionMatrix1 = glm::mat4(1.0f);   

	glm::mat4 modelMatrix1 = glm::mat4(1.0f); 
	modelMatrix1 = glm::translate(modelMatrix1, glm::vec3(1.0f, 0.0f, 0.0f)); 

	glm::mat4 viewMatrix1 = glm::mat4(1.0f);     
	projectionMatrix1 = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);     
	

	//Watermelone
	glm::mat4 projectionMatrix2 = glm::mat4(1.0f);

	glm::mat4 modelMatrix2 = glm::mat4(1.0f);
	modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(-1.0f, 0.0f, 0.0f));

	glm::mat4 viewMatrix2 = glm::mat4(1.0f);
	projectionMatrix2 = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);

	//Triangle
	glm::mat4 projectionMatrix3 = glm::mat4(1.0f);
	glm::mat4 modelMatrix3 = glm::mat4(1.0f);
	glm::mat4 viewMatrix3 = glm::mat4(1.0f);
	projectionMatrix3 = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);


	//Drawing done in the game loop
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }


		//Code here
		glClear(GL_COLOR_BUFFER_BIT);     

		glUseProgram(program2.programID);
		//Triangle
		program2.SetModelMatrix(modelMatrix3);
		program2.SetProjectionMatrix(projectionMatrix3);
		program2.SetViewMatrix(viewMatrix3);
		
		//Class 4, Vertex Triangles
		//For Vertex Triangle creation
		
		float vertices0[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };         
		//Set to program 2
		glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices0);        
		glEnableVertexAttribArray(program2.positionAttribute);        
		glDrawArrays(GL_TRIANGLES, 0, 3);         
		glDisableVertexAttribArray(program2.positionAttribute);     
		
		program2.SetColor(0.2f, 0.8f, 0.4f, 1.0f);

		//Class 5, Load textures

		glUseProgram(program.programID);

		//Draw starman

		//Starman
		program.SetModelMatrix(modelMatrix1);
		program.SetProjectionMatrix(projectionMatrix1);
		program.SetViewMatrix(viewMatrix1);

		glBindTexture(GL_TEXTURE_2D, starTexture);
		
		glBindTexture(GL_TEXTURE_2D, starTexture);
		float vertices1[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);         
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords1[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };         
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1);         
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		

		glDisableVertexAttribArray(program.positionAttribute);         
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Draw Watermelone

		//Watermelone
		program.SetModelMatrix(modelMatrix2);
		program.SetProjectionMatrix(projectionMatrix2);
		program.SetViewMatrix(viewMatrix2);

		glBindTexture(GL_TEXTURE_2D, watermeloneTexture);

		glBindTexture(GL_TEXTURE_2D, watermeloneTexture);
		float vertices2[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);



		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);



		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);

		/*
		//Color Test
		glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
		program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
		*/

		
        SDL_GL_SwapWindow(displayWindow);
    }
    

    SDL_Quit();
    return 0;
}

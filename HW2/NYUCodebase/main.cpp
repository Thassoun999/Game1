#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#define PI 	3.14159265358979323846f

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"
#include "math.h"
#include <iostream>

class Entity;
SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 projectionMatrix;
glm::mat4 modelMatrix1;
glm::mat4 modelMatrix2;
glm::mat4 ballModelMatrix;
glm::mat4 viewMatrix;
void Setup();
void ProcessEvents();
void Update();
void Render();
bool done = false;
SDL_Event event;
int score1 = 0;
int score2 = 0;
float lastFrameTicks;
float elapsed;
float ticks;
int width;
int height;
float projectionHeight;
float projectionDepth;
float projectionWidth;
float aspectRatio;

float ballPosX1 = 0.05f;
float ballPosY1 = 0.05f;
float ballPosX2 = -0.05f;
float ballPosY2 = -0.05f;
float angle = 45.0f;
float ballDirectionX = cos(angle * PI / 180.0f);
float ballDirectionY = sin(angle * PI / 180.0f);

//Reserve 1 for left
float direction1UP = 0.3f;
float direction1DOWN = -0.3f;

//Reserve 2 for Right
float direction2UP = 0.3f;
float direction2DOWN = -0.3f;

int dirswap;

void Setup() {
	width = 1280;
	height = 800;
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	aspectRatio = (float)width / (float)height; //of screen
	projectionHeight = 2.0f;
	projectionWidth = projectionHeight * aspectRatio;
	projectionDepth = 2.0f;
	

#ifdef _WINDOWS
	glewInit();
#endif

	//Setup
	// Setup Before the Loop

	glViewport(0, 0, width, height);


	//Untextured
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	float lastFrameTicks = 0.0f;

	//Blend Here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//Rectangle
	projectionMatrix = glm::mat4(1.0f);
	modelMatrix1 = glm::mat4(1.0f);
	modelMatrix2 = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-projectionWidth, projectionWidth, -projectionHeight, projectionHeight, -projectionDepth, projectionDepth);

	//Ball
	ballModelMatrix = glm::mat4(1.0f);

	
}

void ProcessEvents() {
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}

		else if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				// DO AN ACTION WHEN SPACE IS PRESSED!
				ballPosX1 = 0.05f;
				ballPosY1 = 0.05f;
				ballPosX2 = -0.05f;
				ballPosY2 = -0.05f;
				score1 = 0;
				score2 = 0;
				angle = 45.0f;
				ballDirectionX = cos(angle * PI / 180.0f);
				ballDirectionY = sin(angle * PI / 180.0f);
				ballModelMatrix = glm::mat4(1.0f);
			}	
		}
	}

	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_UP]) {
		// right rectangle go up
		if(direction2UP + 3*elapsed <= projectionHeight){
			
			direction2UP += 3 * elapsed;
			direction2DOWN += 3 * elapsed;
			modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(0.0f, 3 * elapsed, 0.0f));
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		// right rectangle go down
		if (direction2DOWN - 3 * elapsed >= -projectionHeight) {
			direction2UP -= 3 * elapsed;
			direction2DOWN -= 3 * elapsed;
			modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(0.0f, -(3 * elapsed), 0.0f));
		}
		
	}

	if (keys[SDL_SCANCODE_W]) {
		// left rectangle go up
		if (direction1UP + 3 * elapsed <= projectionHeight) {
			direction1UP += 3 * elapsed;
			direction1DOWN += 3 * elapsed;
			modelMatrix1 = glm::translate(modelMatrix1, glm::vec3(0.0f, 3 * elapsed, 0.0f));
		}
	}
	else if (keys[SDL_SCANCODE_S]) {
		// left rectangle go down
		if (direction1DOWN - 3 * elapsed >= -projectionHeight) {
			direction1UP -= 3 * elapsed;
			direction1DOWN -= 3 * elapsed;
			modelMatrix1 = glm::translate(modelMatrix1, glm::vec3(0.0f, -(3 * elapsed), 0.0f));
		}
	}
}

void Update() {

	//Track ball movement and deal with collisions!!! + Wins

	//Wall Collisions
	if ((ballPosY1 + ballDirectionY * elapsed * 4.0f) > projectionHeight) {
		if (angle == 45.0f) {
			angle = 315.0f;
		}
		else {
			angle = 225.0f;
		}
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);
	}
	else if ((ballPosY2 + ballDirectionY * elapsed * 4.0f) < -projectionHeight) {
		if (angle == 225.0f) {
			angle = 135.0f;
		}
		else {
			angle = 45.0f;
		}
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);
	}

	if (ballPosX1 >= projectionWidth) {
		score1++;
		if (score1 == 9) {
			std::cout << "Player 1 Wins!" << std::endl;
			score1 = 0;
			score2 = 0;
		}
		angle = 45.0f;
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);

		ballPosX1 = 0.05f;
		ballPosY1 = 0.05f;
		ballPosX2 = -0.05f;
		ballPosY2 = -0.05f;

		ballModelMatrix = glm::mat4(1.0f);
	}
	else if (ballPosX2 <= -projectionWidth) {
		score2++;
		if (score2 == 9) {
			std::cout << "Player 2 Wins!" << std::endl;
			score1 = 0;
			score2 = 0;
		}
		angle = 45.0f;
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);

		ballPosX1 = 0.05f;
		ballPosY1 = 0.05f;
		ballPosX2 = -0.05f;
		ballPosY2 = -0.05f;

		ballModelMatrix = glm::mat4(1.0f);
	}

	//Paddle Collisions

	//Used as a pre-check
	float direction_X = ballDirectionX * elapsed * 4.0f;
	float direction_Y = ballDirectionY * elapsed * 4.0f;

	//Paddles
	float h1 = 0.6f;
	float w1 = 0.05f;

	//Ball
	float h2 = 0.1f;
	float w2 = 0.1f;

	//Distance Check Paddle Left then Right
	
	//Check and make sure which is rectangle 1 and which is rectangle 2
	float xdistLeft = abs(((ballPosX1 + direction_X) + (ballPosX2 + direction_X)) / 2.0f - (-2.975f));
	float ydistLeft = abs(((ballPosY1 + direction_Y) + (ballPosY2 + direction_Y)) / 2.0f - (direction1UP + direction1DOWN) / 2.0f);
	float xdistRight = abs(((ballPosX1 + direction_X) + (ballPosX2 + direction_X)) / 2.0f - (2.975f));
	float ydistRight = abs(((ballPosY1 + direction_Y) + (ballPosY2 + direction_Y)) / 2.0f - (direction2UP + direction2DOWN) / 2.0f);

	float p1left = xdistLeft - ((w1 + w2) / 2.0f);
	float p2left = ydistLeft - ((h1 + h2) / 2.0f);

	float p1right = xdistRight - ((w1 + w2) / 2.0f);
	float p2right = ydistRight - ((h1 + h2) / 2.0f);

	if ((p1left < 0 && p2left < 0) || (p1right < 0 && p2right < 0)) {
		//Check if left or right paddle is being hit!
		if (p1left < 0 && p2left < 0) {
			if (angle == 225.0f) {
				angle = 315.0f;
			}
			else if (angle == 135.0f) {
				angle = 45.0f;
			}
		}
		else {
			if (angle == 45.0f) {
				angle = 135.0f;;
			}
			else if (angle == 315.0f) {
				angle = 225.0f;
			}
		}
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);
	}

	

	//Calculation
	direction_X = ballDirectionX * elapsed * 4.0f;
	direction_Y = ballDirectionY * elapsed * 4.0f;

	ballModelMatrix = glm::translate(ballModelMatrix, glm::vec3(direction_X, direction_Y, 0.0f));

	ballPosX1 += direction_X;
	ballPosX2 += direction_X;
	ballPosY1 += direction_Y;
	ballPosY2 += direction_Y;
	
}

void Render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Set the items in our shader
	program.SetModelMatrix(modelMatrix1);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);

	//Rectangle 1
	float vertices0[] = { -3.0f, -0.3f, -2.95f, -0.3f, -2.95f, 0.3f, -3.0f, -0.3f, -2.95f, 0.3f, -3.0f, 0.3f };
	//float vertices0[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices0);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);

	
	//Rectangle 2
	program.SetModelMatrix(modelMatrix2);
	float vertices1[] = { 3.0f, -0.3f, 2.95f, -0.3f, 2.95f, 0.3f, 3.0f, -0.3f, 2.95f, 0.3f, 3.0f, 0.3f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	program.SetColor(0.8f, 0.2f, 0.6f, 1.0f);

	//Ball Center
	program.SetModelMatrix(ballModelMatrix);
	float vertices2[] = { -0.05f, -0.05f, 0.05f, -0.05f, 0.05f, 0.05f, -0.05f, -0.05f, 0.05f, 0.05f, -0.05f, 0.05f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	
	
	


}

int main(int argc, char *argv[])
{
    
	Setup();

	float lastFrameTicks = 0.0f;
	float direction1 = 0.0f;
	float direction2 = 0.0f;

    while (!done) {

		ProcessEvents();
		Update();
		Render();

		
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

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
void resetBall();
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

float angle = 45.0f;
float ballDirectionX = cos(angle * PI / 180.0f);
float ballDirectionY = sin(angle * PI / 180.0f);

bool player1Won = false;
bool player2Won = false;


float leftPaddleY = 0.0f;
float leftPaddleX = -3.0f;

float rightPaddleY = 0.0f;
float rightPaddleX = 3.0f;

float ballX = 0.0f;
float ballY = 0.0f;

//float vertices0[] =  { -0.05f, -0.3f, 0.05f, -0.3f, 0.05f, 0.3f, -0.05f, -0.3f, 0.05f, 0.3f, -0.05f, 0.3f };
float paddleHeight = 0.6f;
float paddleWidth = 0.1f;

float ballWidth = 0.1f;
float ballHeight = 0.1f;

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
				// DO AN ACTION WHEN SPACE IS PRESSED! Reset the game!
				resetBall();
				score1 = 0;
				score2 = 0;
				player1Won = false;
				player2Won = false;
				angle = 45.0f;
				ballDirectionX = cos(angle * PI / 180.0f);
				ballDirectionY = sin(angle * PI / 180.0f);
			}	
		}
	}

	ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_UP]) {
		// right rectangle go up
		if((rightPaddleY + paddleHeight / 2) <= projectionHeight){
			rightPaddleY += 3 * elapsed;
		}
		
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
		// right rectangle go down
		if ((rightPaddleY - paddleHeight / 2) >= -projectionHeight) {
			rightPaddleY -= 3 * elapsed;
		}
		
	}

	if (keys[SDL_SCANCODE_W]) {
		// left rectangle go up
		if ((leftPaddleY + paddleHeight / 2) <= projectionHeight) {
			leftPaddleY += 3 * elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_S]) {
		// left rectangle go down
		if ((leftPaddleY - paddleHeight / 2) >= -projectionHeight) {
			leftPaddleY -= 3 * elapsed;
		}
	}
}

void resetBall() {
	ballX = 0.0f;
	ballY = 0.0f;
}


void Update() {

	//move ball
	ballX += ballDirectionX * elapsed * 5.0f;
	ballY +=  ballDirectionY * elapsed * 5.0f;

	//Track ball movement and deal with collisions!!! + Wins

	//Wall Collisions
	if (ballY + (ballHeight / 2) > projectionHeight) {
		if (angle == 45.0f) {
			angle = 315.0f;
		}
		else {
			angle = 225.0f;
		}
		ballDirectionY = sin(angle * PI / 180.0f);
	}
	else if (ballY - (ballHeight / 2) < -projectionHeight) {
		if (angle == 225.0f) {
			angle = 135.0f;
		}
		else {
			angle = 45.0f;
		}
		ballDirectionY = sin(angle * PI / 180.0f);
	}

	if (ballX + (ballWidth /2) >= projectionWidth) {
		score1++;
		if (score1 == 3) {
			std::cout << "Player 1 Wins!" << std::endl;
			player1Won = true;
			player2Won = false;
			
			score1 = 0;
			score2 = 0;
		}
		angle = 45.0f;
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);

		
		resetBall();
		
	}
	else if (ballX - (ballWidth / 2) <= -projectionWidth) {
		score2++;
		if (score2 == 3) {
			std::cout << "Player 2 Wins!" << std::endl;

			player2Won = true;
			player1Won = false;

			score1 = 0;
			score2 = 0;
		}
		angle = 45.0f;
		ballDirectionX = cos(angle * PI / 180.0f);
		ballDirectionY = sin(angle * PI / 180.0f);

		resetBall();

		
	}

	//Paddle Collisions

	//Distance Check Paddle Left then Right
	
	//Check and make sure which is rectangle 1 and which is rectangle 2
	float p1left = abs(leftPaddleX - ballX) - ((paddleWidth + ballWidth) / 2.0f);
	float p2left = abs(leftPaddleY - ballY) - ((paddleHeight + ballHeight) / 2.0f);

	float p1right = abs(rightPaddleX - ballX) - ((paddleWidth + ballWidth) / 2.0f);
	float p2right = abs(rightPaddleY - ballY) - ((paddleHeight + ballHeight) / 2.0f);

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
	
}

void Render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Set the items in our shader
	program.SetModelMatrix(modelMatrix1);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	if (!(player1Won)) {
		program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
	}
	else {
		program.SetColor(0.3f, 0.3f, 0.9f, 1.0f);
	}
	
	

	//Rectangle 1
	float vertices0[] = { -0.05f, -0.3f, 0.05f, -0.3f, 0.05f, 0.3f, -0.05f, -0.3f, 0.05f, 0.3f, -0.05f, 0.3f };
	
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices0);
	glEnableVertexAttribArray(program.positionAttribute);
	modelMatrix1 = glm::mat4(1.0f);
	modelMatrix1 = glm::translate(modelMatrix1, glm::vec3(leftPaddleX, leftPaddleY, 0.0f));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	
	if (!(player2Won)) {
		program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
	}
	else {
		program.SetColor(0.3f, 0.3f, 0.9f, 1.0f);
	}


	
	//Rectangle 2
	program.SetModelMatrix(modelMatrix2);
	float vertices1[] = { -0.05f, -0.3f, 0.05f, -0.3f, 0.05f, 0.3f, -0.05f, -0.3f, 0.05f, 0.3f, -0.05f, 0.3f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
	glEnableVertexAttribArray(program.positionAttribute);
	modelMatrix2 = glm::mat4(1.0f);
	modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(rightPaddleX, rightPaddleY, 0.0f));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);

	program.SetColor(0.8f, 0.2f, 0.6f, 1.0f);

	//Ball Center
	program.SetModelMatrix(ballModelMatrix);
	float vertices2[] = { -0.05f, -0.05f, 0.05f, -0.05f, 0.05f, 0.05f, -0.05f, -0.05f, 0.05f, 0.05f, -0.05f, 0.05f };
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
	glEnableVertexAttribArray(program.positionAttribute);
	ballModelMatrix = glm::mat4(1.0f);
	ballModelMatrix = glm::translate(ballModelMatrix, glm::vec3(ballX, ballY, 0.0f));
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

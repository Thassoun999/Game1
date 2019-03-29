#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION
#define MAX_BULLETS_SHIP 2
#define MAX_BULLETS_ALIEN 4
#define MAX_ALIENS 5
#define ALIEN_TYPES 4

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
#include <vector>
#include <stdlib.h> 

class SheetSprite;
class Entity;
enum GameMode{STATE_MAIN_MENU, STATE_GAME_SCREEN};


//Game state is a struct with all the entities

SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;

float time = 0.0f;


void Setup();
void ProcessEvents();
void Update();
void Render();
void resetGame();
GLuint LoadTexture(const char *filepath);
void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing, float xcenter, float ycenter);

bool done = false;
SDL_Event event;

float lastFrameTicks;
float elapsed;
float ticks;
int screenwidth;
int screenheight;
float projectionHeight;
float projectionDepth;
float projectionWidth;
float aspectRatio;

float aliensLeftBound = -2.5f;
float aliensRightBound = 1.5f;
bool goingRight = true;
int downmove = 0;
int alienAmount = MAX_ALIENS * ALIEN_TYPES;
std::string commandprint = "Press Return!";
std::string titleprint = "Space Invaders";
std::string gameoverprint = "Game Over!";
std::string hpprint = "HP:";
std::string one = "1";
std::string two = "2";
std::string three = "3";
bool gameOver = false;

int onScreenAlienBullets = 0;

GLuint txtTexture;
GLuint alienTexture;
GLuint shipTexture;
GLuint bulletTexture;

class SheetSprite {
public:
	SheetSprite() {};
	//Of the sprite in the spritesheet altogether!!!
	SheetSprite(unsigned int textureIDin, float uin, float vin, float widthin, float heightin, float sizein) {
		textureID = textureIDin;
		size = sizein;
		u = uin;
		v = vin;
		width = widthin;
		height = heightin;
	};
	
	
	float size = 1.0f;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};


class Entity {
public:
	void Draw(ShaderProgram &program);
	//void uniDraw(ShaderProgram &program);

	glm::vec3 position;
	glm::vec3 size;

	//glm::vec3 velocity;
	//float rotation;

	SheetSprite sprite;
	bool shot = false;
	int health;

};

void Entity::Draw(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, sprite.textureID);

	GLfloat texCoords[] = {
		sprite.u, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v,
		sprite.u, sprite.v,
		sprite.u + sprite.width, sprite.v,
		sprite.u, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v + sprite.height
	};

	float aspect = sprite.width / sprite.height;
	//Might want to have this specified or something
	float vertices[] = {
		-0.5f * sprite.size * aspect, -0.5f * sprite.size,
		0.5f * sprite.size * aspect, 0.5f * sprite.size,
		-0.5f * sprite.size * aspect, 0.5f * sprite.size,
		0.5f * sprite.size * aspect, 0.5f * sprite.size,
		-0.5f * sprite.size * aspect, -0.5f * sprite.size,
		0.5f * sprite.size * aspect, -0.5f * sprite.size
	};

	//Drawing happens here

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	program.SetModelMatrix(modelMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}
//Using this code for text draw

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing, float xcenter, float ycenter) {
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	float spriteWidth = 1.0f / (float)32.0f;
	float spriteHeight = 1.0f / (float)4.0f;

	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 32) / 32.0f;
		float texture_y = (float)(spriteIndex / 32) / 4.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,

		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + spriteHeight,
			texture_x + spriteWidth, texture_y,
			texture_x + spriteWidth, texture_y + spriteHeight,
			texture_x + spriteWidth, texture_y,
			texture_x, texture_y + spriteHeight,
		});
	}

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(xcenter, ycenter, 0.0f));
	program.SetModelMatrix(modelMatrix);

	glDrawArrays(GL_TRIANGLES, 0, text.size()*6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

//Put all those inside of a game state
class GameState {
public:
	GameState() {
		std::vector<Entity> al1(MAX_ALIENS);
		std::vector<Entity> al2(MAX_ALIENS);
		std::vector<Entity> al3(MAX_ALIENS);
		std::vector<Entity> al4(MAX_ALIENS);

		alienShips1 = al1;
		alienShips2 = al2;
		alienShips3 = al3;
		alienShips4 = al4;

		std::vector<Entity> sb(MAX_BULLETS_SHIP);
		std::vector<Entity> ab(MAX_BULLETS_ALIEN);

		shipBullets = sb;
		alienBullets = ab;

	}
	Entity mainShip;
	Entity exclamation;

	std::vector<Entity> alienShips1;
	std::vector<Entity> alienShips2;
	std::vector<Entity> alienShips3;
	std::vector<Entity> alienShips4;

	std::vector<Entity> shipBullets;
	std::vector<Entity> alienBullets;
};

GameState state;
GameMode mode = STATE_MAIN_MENU;



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

void resetGame() {
	state.mainShip.position = glm::vec3(0.0f, -1.85f, 0.0f);
	state.mainShip.health = 3;
	for (int i = 0; i < ALIEN_TYPES; i++) {
		for (int y = 0; y < MAX_ALIENS; y++) {
			if (i == 0) {
				state.alienShips1[y].position = glm::vec3(-2.5f + (float)y, 1.4f, 0.0f);
				state.alienShips1[y].health = 1;
				
			}
			else if (i == 1) {
				state.alienShips2[y].position = glm::vec3(-2.5f + (float)y, 1.0f, 0.0f);
				state.alienShips2[y].health = 1;
			}
			else if (i == 2) {
				state.alienShips3[y].position = glm::vec3(-2.5f + (float)y, 0.6f, 0.0f);
				state.alienShips3[y].health = 1;
			}
			else if (i == 3) {
				state.alienShips4[y].position = glm::vec3(-2.5f + (float)y, 0.2f, 0.0f);
				state.alienShips4[y].health = 1;
			}
		}
	}

	aliensLeftBound = -2.5f;
	aliensRightBound = 1.5f;
	downmove = 0;
	goingRight = true;
	alienAmount = MAX_ALIENS * ALIEN_TYPES;
	for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
		state.alienBullets[i].shot = false;
	}
	for (int i = 0; i < MAX_BULLETS_SHIP; i++) {
		state.shipBullets[i].shot = false;
	}

}

void Setup() {
	screenwidth = 1280;
	screenheight = 800;
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenwidth, screenheight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

	aspectRatio = (float)screenwidth / (float)screenheight; //of screen
	projectionHeight = 2.0f;
	projectionWidth = projectionHeight * aspectRatio;
	projectionDepth = 2.0f;
	

#ifdef _WINDOWS
	glewInit();
#endif

	//Setup
	// Setup Before the Loop

	glViewport(0, 0, screenwidth, screenheight);


	//Untextured
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	float lastFrameTicks = 0.0f;

	//Blend Here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Get my TextureIDs
	txtTexture = LoadTexture(RESOURCE_FOLDER "BlockFont.png"); //Find another thing
	shipTexture = LoadTexture(RESOURCE_FOLDER "sheet.png");

	//Overall setup
	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-projectionWidth, projectionWidth, -projectionHeight, projectionHeight, -projectionDepth, projectionDepth);

	//Make my sprites and entities
	std::cout << mode << std::endl;
	//Size of x is 0.5 * size * aspect while size of y is 0.5 * size
	//This is given that aspect is width * height

	//<SubTexture name="playerShip3_green.png" x="346" y="75" width="98" height="75"/>
	state.mainShip.sprite = SheetSprite(shipTexture, 346.0f / 1024.0f, 75.0f / 1024.0f, 98.0f / 1024.0f, 75.0f / 1024.0f, 0.25f);
	state.mainShip.position = glm::vec3(0.0f, -1.85f, 0.0f);
	state.mainShip.size = glm::vec3(0.5f * 0.25f * (98.0f / 1024.0f)/(75.0f / 1024.0f), 0.5f * 0.25f, 0.0f);
	state.mainShip.health = 3;

	
	//Aliens
	//468 x 39
	for (int i = 0; i < ALIEN_TYPES; i++) {
		for (int y = 0; y < MAX_ALIENS; y++) {
			if (i == 0) {
				//<SubTexture name="enemyBlack1.png" x="423" y="728" width="93" height="84"/>
				state.alienShips1[y].sprite = SheetSprite(shipTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);
				state.alienShips1[y].position = glm::vec3(-2.5f + (float)y, 1.4f, 0.0f);
				state.alienShips1[y].size = glm::vec3(0.5f * 0.25f * (423.0f / 1024.0f) / (728.0f / 1024.0f), 0.5f * 0.25f, 0.0f);
				state.alienShips1[y].health = 1;
			}
			else if (i == 1) {
				//<SubTexture name="enemyGreen1.png" x="425" y="552" width="93" height="84"/>
				state.alienShips2[y].sprite = SheetSprite(shipTexture, 425.0f / 1024.0f, 552.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);
				state.alienShips2[y].position = glm::vec3(-2.5f + (float)y, 1.0f, 0.0f);
				state.alienShips2[y].size = glm::vec3(0.5f * 0.25f * (425.0f / 1024.0f) / (552.0f / 1024.0f), 0.5f * 0.25f, 0.0f);
				state.alienShips2[y].health = 1;
			}
			else if (i == 2) {
				//<SubTexture name="enemyRed1.png" x="425" y="384" width="93" height="84"/>
				state.alienShips3[y].sprite = SheetSprite(shipTexture, 425.0f / 1024.0f, 384.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);
				state.alienShips3[y].position = glm::vec3(-2.5f + (float)y, 0.6f, 0.0f);
				state.alienShips3[y].size = glm::vec3(0.5f * 0.25f * (425.0f / 1024.0f) / (384.0f / 1024.0f), 0.5f * 0.25f, 0.0f);
				state.alienShips3[y].health = 1;
			}
			else if (i == 3) {
				//<SubTexture name="enemyBlue1.png" x="425" y="468" width="93" height="84"/>
				state.alienShips4[y].sprite = SheetSprite(shipTexture, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.25f);
				state.alienShips4[y].position = glm::vec3(-2.5f + (float)y, 0.2f, 0.0f);
				state.alienShips4[y].size = glm::vec3(0.5f * 0.25f * (425.0f / 1024.0f) / (468.0f / 1024.0f), 0.5f * 0.25f, 0.0f);
				state.alienShips4[y].health = 1;
			}
		}
	}
	
	//Bullets Ship
	//<SubTexture name = "laserGreen03.png" x = "855" y = "173" width = "9" height = "57" / >
	for (int i = 0; i < MAX_BULLETS_SHIP; i++) {
		state.shipBullets[i].sprite = SheetSprite(shipTexture, 855.0f / 1024.0f, 173.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.3f);
		state.shipBullets[i].position = glm::vec3(-500.0f, -500.0f, 0.0f);
		state.shipBullets[i].size = glm::vec3(0.5f * 0.3f * (9.0f / 1024.0f) /(57.0f / 1024.0f), 0.5f * 0.3f, 0.0f);
	}

	//Bullets Aliens
	
	
	//<SubTexture name="laserRed15.png" x="856" y="926" width="9" height="57"/>
	for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
		state.alienBullets[i].sprite = SheetSprite(shipTexture, 856.0f / 1024.0f, 926.0f / 1024.0f, 9.0f / 1024.0f, 57.0f / 1024.0f, 0.3f);
		state.alienBullets[i].position = glm::vec3(-500.0f, -500.0f, 0.0f);
		
		state.alienBullets[i].size = glm::vec3(0.5f * 0.3f * (856.0f / 1024.0f)/(926.0f / 1024.0f), 0.5f * 0.3f, 0.0f);
	}
	
}

void ProcessEvents() {
	switch (mode) {
		case STATE_MAIN_MENU:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						resetGame();
						mode = STATE_GAME_SCREEN;
					
					}
				}
			}
			ticks = (float)SDL_GetTicks() / 1000.0f;
			elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;
		break;
		case STATE_GAME_SCREEN:
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}

				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {


						for (int i = 0; i < MAX_BULLETS_SHIP; i++) {

							if (state.shipBullets[i].shot == false) {

								state.shipBullets[i].position = state.mainShip.position + glm::vec3(0.0f, 0.05f, 0.0f);
								state.shipBullets[i].shot = true;

								break;
							}
						}

					}
				}
			}

			ticks = (float)SDL_GetTicks() / 1000.0f;
			elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			//Ship movement
			if (keys[SDL_SCANCODE_RIGHT]) {
				//Ship goes right
				if (state.mainShip.position.x + (state.mainShip.size.x / 2) < projectionWidth) {
					state.mainShip.position.x += 2.0f * elapsed;
				}
			}
			else if (keys[SDL_SCANCODE_LEFT]) {
				//Ship goes left
				if (state.mainShip.position.x - (state.mainShip.size.x / 2) > -projectionWidth) {
					state.mainShip.position.x -= 2.0f * elapsed;
				}
			}
		break;
	}
	
}


void Update() {
	switch (mode) {
		case STATE_MAIN_MENU:
			resetGame();
		break;
		case STATE_GAME_SCREEN:
			//Alien movement, Alien Shooting, Bullet Movement, Bullet Collision

			//0) Check Win and Loss
			if (alienAmount == 0) {
				mode = STATE_MAIN_MENU;
				gameOver = true;
			}

			if (state.mainShip.health == 0) {
				mode = STATE_MAIN_MENU;
				gameOver = true;
			}


			//1) Ship shooting

			//a)Out of bounds check, we can allow the bullet to keep on going
			for (int i = 0; i < MAX_BULLETS_SHIP; i++) {
				if (state.shipBullets[i].position.y > projectionHeight + 0.2f) {

					state.shipBullets[i].shot = false;
					state.shipBullets[i].position = glm::vec3(-500.0f, -500.0f, 0.0f);
				}
			}

			//b)Ship Shooting
			for (int i = 0; i < MAX_BULLETS_SHIP; i++) {
				if (state.shipBullets[i].shot) state.shipBullets[i].position.y += 3.0f * elapsed;
			}

			//c)Ship Bullet Collision Check

			//For each bullet
			for (int bullet = 0; bullet < MAX_BULLETS_SHIP; bullet++) {
				//For each ship
				for (int row = 0; row < ALIEN_TYPES; row++) {
					for (int col = 0; col < MAX_ALIENS; col++) {
						if (row == 0) {
							float p1 = abs(state.shipBullets[bullet].position.x - state.alienShips1[col].position.x) - ((state.shipBullets[bullet].size.x + state.alienShips1[col].size.x) / 2.0f);
							float p2 = abs(state.shipBullets[bullet].position.y - state.alienShips1[col].position.y) - ((state.shipBullets[bullet].size.y + state.alienShips1[col].size.y) / 2.0f);
							if (p1 < 0 && p2 < 0) {
								state.shipBullets[bullet].shot = false;
								state.shipBullets[bullet].position = glm::vec3(-500.0f, -500.0f, 0.0f);
								state.alienShips1[col].health = 0;
								state.alienShips1[col].position = glm::vec3(500.0f, 500.0f, 0.0f);
								alienAmount--;

							}
						}
						else if (row == 1) {
							float p1 = abs(state.shipBullets[bullet].position.x - state.alienShips2[col].position.x) - ((state.shipBullets[bullet].size.x + state.alienShips2[col].size.x) / 2.0f);
							float p2 = abs(state.shipBullets[bullet].position.y - state.alienShips2[col].position.y) - ((state.shipBullets[bullet].size.y + state.alienShips2[col].size.y) / 2.0f);
							if (p1 < 0 && p2 < 0) {
								state.shipBullets[bullet].shot = false;
								state.shipBullets[bullet].position = glm::vec3(-500.0f, -500.0f, 0.0f);
								state.alienShips2[col].position = glm::vec3(500.0f, 500.0f, 0.0f);
								state.alienShips2[col].health = 0;
								alienAmount--;

							}
						}
						else if (row == 2) {
							float p1 = abs(state.shipBullets[bullet].position.x - state.alienShips3[col].position.x) - ((state.shipBullets[bullet].size.x + state.alienShips3[col].size.x) / 2.0f);
							float p2 = abs(state.shipBullets[bullet].position.y - state.alienShips3[col].position.y) - ((state.shipBullets[bullet].size.y + state.alienShips3[col].size.y) / 2.0f);
							if (p1 < 0 && p2 < 0) {
								state.shipBullets[bullet].shot = false;
								state.shipBullets[bullet].position = glm::vec3(-500.0f, -500.0f, 0.0f);
								state.alienShips3[col].position = glm::vec3(500.0f, 500.0f, 0.0f);
								state.alienShips3[col].health = 0;
								alienAmount--;

							}
						}
						else if (row == 3) {
							float p1 = abs(state.shipBullets[bullet].position.x - state.alienShips4[col].position.x) - ((state.shipBullets[bullet].size.x + state.alienShips4[col].size.x) / 2.0f);
							float p2 = abs(state.shipBullets[bullet].position.y - state.alienShips4[col].position.y) - ((state.shipBullets[bullet].size.y + state.alienShips4[col].size.y) / 2.0f);
							if (p1 < 0 && p2 < 0) {
								state.shipBullets[bullet].shot = false;
								state.shipBullets[bullet].position = glm::vec3(-500.0f, -500.0f, 0.0f);
								state.alienShips4[col].position = glm::vec3(500.0f, 500.0f, 0.0f);
								state.alienShips4[col].health = 0;
								alienAmount--;

							}
						}
					}
				}
			}
			//2) Alien Movement
			//a) Regular Movement
			if (goingRight) {
				for (int y = 0; y < MAX_ALIENS; y++) {
					state.alienShips1[y].position.x += 0.3f * elapsed;
					state.alienShips2[y].position.x += 0.3f * elapsed;
					state.alienShips3[y].position.x += 0.3f * elapsed;
					state.alienShips4[y].position.x += 0.3f * elapsed;
				}


				aliensRightBound += 0.3f * elapsed;
				aliensLeftBound += 0.3f * elapsed;

				if (aliensRightBound > projectionWidth) {
					goingRight = false;
					downmove++;
					for (int y = 0; y < MAX_ALIENS; y++) {
						state.alienShips1[y].position.y -= 0.3f;
						state.alienShips2[y].position.y -= 0.3f;
						state.alienShips3[y].position.y -= 0.3f;
						state.alienShips4[y].position.y -= 0.3f;
					}
				}
			}
			else {
				for (int y = 0; y < MAX_ALIENS; y++) {
					state.alienShips1[y].position.x -= 0.3f * elapsed;
					state.alienShips2[y].position.x -= 0.3f * elapsed;
					state.alienShips3[y].position.x -= 0.3f * elapsed;
					state.alienShips4[y].position.x -= 0.3f * elapsed;
				}


				aliensRightBound -= 0.3f * elapsed;
				aliensLeftBound -= 0.3f * elapsed;

				if (aliensLeftBound < -projectionWidth) {
					goingRight = true;
					downmove++;
					for (int y = 0; y < MAX_ALIENS; y++) {
						state.alienShips1[y].position.y -= 0.3f;
						state.alienShips2[y].position.y -= 0.3f;
						state.alienShips3[y].position.y -= 0.3f;
						state.alienShips4[y].position.y -= 0.3f;
					}
				}
			}

			//b) Vertical check (is equal to 8 downs)
			if (downmove == 7) {
				//GAMEOVER!
				state.mainShip.health = 0;
			}

			//3) Alien Shooting

			//a)When shooting happens, every 3 seconds

			time += elapsed;
			if (time > 1.5f) {
				time = 0.0f;
				int randi = rand() % ALIEN_TYPES;
				int randy = rand() % MAX_ALIENS;

				if (randi == 0) {
					for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
						if (state.alienBullets[i].shot == false) {
							//mainShip.position + glm::vec3(0.0f, 0.05f, 0.0f);
							state.alienBullets[i].position = state.alienShips1[randy].position + glm::vec3(0.0f, -0.05f, 0.0f);
							state.alienBullets[i].shot = true;
							break;
						}
					}
				}
				else if (randi == 1) {
					for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
						if (state.alienBullets[i].shot == false) {
							//mainShip.position + glm::vec3(0.0f, 0.05f, 0.0f);
							state.alienBullets[i].position = state.alienShips2[randy].position + glm::vec3(0.0f, -0.05f, 0.0f);
							state.alienBullets[i].shot = true;
							break;
						}
					}
				}
				else if (randi == 2) {
					for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
						if (state.alienBullets[i].shot == false) {
							//mainShip.position + glm::vec3(0.0f, 0.05f, 0.0f);
							state.alienBullets[i].position = state.alienShips3[randy].position + glm::vec3(0.0f, -0.05f, 0.0f);
							state.alienBullets[i].shot = true;
							break;
						}
					}
				}
				else if (randi == 3) {
					for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
						if (state.alienBullets[i].shot == false) {
							//mainShip.position + glm::vec3(0.0f, 0.05f, 0.0f);
							state.alienBullets[i].position = state.alienShips4[randy].position + glm::vec3(0.0f, -0.05f, 0.0f);
							state.alienBullets[i].shot = true;
							break;
						}
					}
				}

			}

			//Out of bounds check, we can allow the bullet to keep on going
			for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
				if (state.alienBullets[i].position.y < -projectionHeight + 0.2f) {

					state.alienBullets[i].shot = false;
					state.alienBullets[i].position = glm::vec3(-500.0f, -500.0f, 0.0f);
				}
			}

			//Alien Shooting
			for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
				if (state.alienBullets[i].shot) state.alienBullets[i].position.y -= 3.0f * elapsed;
			}

			//b) Collision Alien Bullet to Ship (YEEEESH)
			for (int bullet = 0; bullet < MAX_BULLETS_ALIEN; bullet++) {
				//For each ship
				float p1 = abs(state.alienBullets[bullet].position.x - state.mainShip.position.x) - ((state.alienBullets[bullet].size.x + state.mainShip.size.x) / 2.0f);
				float p2 = abs(state.alienBullets[bullet].position.y - state.mainShip.position.y) - ((state.alienBullets[bullet].size.y + state.mainShip.size.y) / 2.0f);
				if (p1 < 0 && p2 < 0) {
					state.alienBullets[bullet].shot = false;
					state.alienBullets[bullet].position = glm::vec3(-500.0f, -500.0f, 0.0f);
					state.mainShip.health--;
				}
			}
		break;
	}
	
}

void Render() {
	switch (mode) {
		case STATE_MAIN_MENU:
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			//Width 576 && Height 128
			//txtTexture
			DrawText(program, txtTexture, titleprint, 0.3f, 0.0f, -2.0f, 0.0f);
			DrawText(program, txtTexture, commandprint, 0.1f, 0.0f, -0.6f, -0.4f);
			if (gameOver) {
				DrawText(program, txtTexture, gameoverprint, 0.1f, 0.0f, -0.45, -0.5f);
			}

		break;
		case STATE_GAME_SCREEN:
			//DRAW EVERYTHING
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			//Draw Sprites via Entities here!

			//Ship and Ship bullets
			state.mainShip.Draw(program);
			if (state.mainShip.health == 1) {
				DrawText(program, txtTexture, hpprint + one, 0.15f, 0.0f, -3.0f, -1.75f);
			}
			else if (state.mainShip.health == 2) {
				DrawText(program, txtTexture, hpprint + two, 0.15f, 0.0f, -3.0f, -1.75f);
			}
			else if (state.mainShip.health == 3) {
				DrawText(program, txtTexture, hpprint + three, 0.15f, 0.0f, -3.0f, -1.75f);
			}
			
			for (int i = 0; i < MAX_BULLETS_SHIP; i++) {
				if (state.shipBullets[i].shot == true) state.shipBullets[i].Draw(program);
			}

			//Aliens and Alien bullets
			for (int i = 0; i < ALIEN_TYPES; i++) {
				for (int y = 0; y < MAX_ALIENS; y++) {
					if (i == 0) {
						if (state.alienShips1[y].health > 0) state.alienShips1[y].Draw(program);
					}
					else if (i == 1) {
						if (state.alienShips2[y].health > 0) state.alienShips2[y].Draw(program);
					}
					else if (i == 2) {
						if (state.alienShips3[y].health > 0) state.alienShips3[y].Draw(program);
					}
					else if (i == 3) {
						if (state.alienShips4[y].health > 0) state.alienShips4[y].Draw(program);
					}
				}
			}

			for (int i = 0; i < MAX_BULLETS_ALIEN; i++) {
				if (state.alienBullets[i].shot == true) state.alienBullets[i].Draw(program);
			}
		break;
	}
	
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


/*
void Entity::uniDraw(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, sprite.textureID);

	GLfloat texCoords[] = {
		sprite.u, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v,
		sprite.u, sprite.v,
		sprite.u + sprite.width, sprite.v,
		sprite.u, sprite.v + sprite.height,
		sprite.u + sprite.width, sprite.v + sprite.height
	};

	float vertices[] = { -0.5f * sprite.size, -0.5f * sprite.size,
		0.5f * sprite.size, 0.5f * sprite.size,
		-0.5f * sprite.size, 0.5f * sprite.size,
		0.5f * sprite.size, 0.5f * sprite.size,
		-0.5f * sprite.size, -0.5f * sprite.size,
		0.5f * sprite.size, -0.5f *sprite.size};

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	program.SetModelMatrix(modelMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}
*/

/*SheetSprite(unsigned int textureIDin, int index, int spriteCountX, int spriteCountY, float sizein) {
		float uin = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
		float vin = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
		float spriteWidth = 1.0f / (float)spriteCountX;
		float spriteHeight = 1.0f / (float)spriteCountY;

		u = uin;
		v = vin;
		width = spriteWidth;
		height = spriteHeight;
		textureID = textureIDin;
		size = sizein;

	}
*/
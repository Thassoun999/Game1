#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <SDL_mixer.h>


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define FIXED_TIMESTEP 0.002f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.2f
#define SPRITE_COUNT_X_TILE 59
#define SPRITE_COUNT_Y_TILE 34 

#define SPRITE_COUNT_X_ITEM 4
#define SPRITE_COUNT_Y_ITEM 4

#define SPRITE_COUNT_X_ENEMY 4
#define SPRITE_COUNT_Y_ENEMY 3

#define SPRITE_COUNT_X_PLAYER 4
#define SPRITE_COUNT_Y_PLAYER 2


#include "ShaderProgram.h"
#include "PerlinNoise.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"
#include "math.h"
#include <vector>
#include <stdlib.h> 
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

ShaderProgram program;
//ShaderProgram program2;

SDL_Window* displayWindow;

//FileReading.h



//256 x 64
int mapWidth;
int mapHeight;
unsigned int** levelData;

bool readHeader(std::ifstream &stream);
void readLayerData(std::ifstream &stream);
void readEntityData(std::ifstream &stream);

std::string commandprint = "Press Return!";
std::string titleprint = "Final Hell!";

std::string gameoverCommandPrint = "Press TAB To Relive Hell!";
std::string gameOver = "Gamer Over! You Died!";
std::string secret = "Secret Shiny Haiku:";
std::string secret1 = "I am overwhelmed,";
std::string secret2 = "I sure hope I get an A,";
std::string secret3 = "Or consequences!";


//For music
int globalTrack = 0;
float deadTime = 0.0f;
bool stardead = false;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_SCREEN, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;


//Entity
class Entity {
public:
	Entity(float posX, float posY) {
		position.x = posX;
		position.y = posY;
	}
	Entity() {}
	virtual void Render(ShaderProgram &program) = 0;
	virtual void setPos(float x, float y) = 0;

	//SheetSprite sprite;
	glm::vec3 position;
	glm::vec3 initialPositions;
	glm::vec3 checkPointPositions;

	glm::vec3 size;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 friction;

	bool isStatic;

	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;

	GLuint textureID;
};

class Player : public Entity {
public:
	Player(float posX, float posY) : Entity(posX, posY) {
		friction.x = 1.00f;
		friction.y = 0.44f;

		acceleration.x = 0.0f;
		acceleration.y = 0.0f;

		velocity.x = 0.0f;
		velocity.y = 0.0f;
	}
	Player() : Entity() {
		friction.x = 1.00f;
		friction.y = 0.44f;

		acceleration.x = 0.0f;
		acceleration.y = 0.0f;

		velocity.x = 0.0f;
		velocity.y = 0.0f;
	}
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
		initialPositions.x = x;
		initialPositions.y = y;
	}
	virtual void Render(ShaderProgram &program);
	virtual void Update(float elapsed);
	virtual void CollisionX();
	virtual void CollisionY();
};

class Enemy : public Entity {
public:
	Enemy(float posX, float posY) : Entity(posX, posY) {
		friction.x = 1.50f;
		friction.y = 0.44f;

		acceleration.x = 0.6f;
		acceleration.y = 0.0f;

		velocity.x = 0.0f;
		velocity.y = 0.0f;
	}
	Enemy() : Entity() {
		friction.x = 1.50f;
		friction.y = 0.44f;

		acceleration.x = 0.6f;
		acceleration.y = 0.0f;

		velocity.x = 0.0f;
		velocity.y = 0.0f;
	}
	std::string name;

	virtual void Render(ShaderProgram &program);
	virtual void Update(float elapsed);
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
	}
	virtual void CollisionX();
	virtual void CollisionY();

	//virtual void Update(float elapsed);
	//virtual bool CollidesWith(Entity &entity);

};

class Item : public Entity {
public:
	Item(float posX, float posY) : Entity(posX, posY) {}
	Item() : Entity() {}
	std::string name;
	int track;
	float hover;
	virtual void Render(ShaderProgram &program);
	virtual void Update(float elapsed);
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
	}
	//virtual bool CollidesWith(Entity &entity);

};



float lerp(float v0, float v1, float t);

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax);

float alphaVar = 0.0f;



//End


bool done = false;
bool gameBoot = true;
bool gotBanan = false;
SDL_Event event;
Player fileGameHero;
std::vector<Enemy> fileGameEnemies;
std::vector<Item> fileGameItems;

int screenwidth;
int screenheight;
float aspectRatio;
float projectionWidth;
float projectionHeight;
float projectionDepth;

float lastFrameTicks;
float accumulator = 0.0f;
float timerItem = 0.0f;
float shinytimerItem = 0.0f;
float timerEnemy = 0.0f;

float screenShake = 0.0f;
bool playerIsHit = false;


bool itemGot = false;
bool playerSwitch = false;
int indexForSprite = 0;
int itemIndex = 0;
int shinyItemIndex = 4;
int slimeIndex = 0;
int grottoIndex = 4;


bool didAJump = false;
bool landedFromAJump = true;

void inFile(std::string documentName);
void Setup();
void ProcessEvents();
void Update(float elapsed);
void Render();
void drawLevelData(unsigned int** levelData);
GLuint LoadTexture(const char *filepath);

GLuint tilesID;
GLuint playerID;
GLuint enemiesID;
GLuint itemsID;
GLuint txtTexture;

glm::mat4 modelMatrix;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

//Sounds
Mix_Chunk* jumpMan;
Mix_Chunk* shineGet;
Mix_Music* musicNow;

//Particles
float animationTime = 0.0f;


//std::vector<int> staticIndex = { 828, 1423, 355, 479, 480, 538, 539, 356, 357, 358, 414, 415, 416, 417, 473, 474, 475, 476, 532, 533, 534, 535, 424, 425, 426, 542, 543, 544, 496, 497, 498, 797, 576, 577, 798, 799, 800, 801, 802, 856, 857, 858, 859, 860, 861, 915, 916, 917, 483, 484, 485 };
//Change here!
std::vector<int> staticIndex = { 863, 862, 858, 864, 601, 602, 603, 483, 484, 485, 496, 497, 498, 542, 543, 544, 424, 425, 426, 479, 480, 538, 539, 355, 356, 357, 358, 414, 415, 416, 417, 473, 474, 475, 476, 532, 533, 534, 535, 501, 442, 606, 607, 608, 488, 489, 490, 429, 430, 431, 547, 548, 549 };
std::vector<int> turnAroundSlime = { 863, 862 };


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

	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

//srcMin and srcMax are start and end times
//dstMin and dstMax are distances!
float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
	float retVal = dstMin + ((value - srcMin) / (srcMax - srcMin) * (dstMax - dstMin));
	if (retVal < dstMin) {
		retVal = dstMin;
	}
	if (retVal > dstMin) {
		retVal = dstMax;
	}
	return retVal;
}

float lerp(float v0, float v1, float t) {
	//std::cout << (1.0f - t)*v0 + t * v1 << std::endl;
	return (1.0f - t)*v0 + t * v1;
}


void resetGame() {
	fileGameHero.position.x = fileGameHero.initialPositions.x;
	fileGameHero.position.y = fileGameHero.initialPositions.y;

	for (size_t i = 0; i < fileGameItems.size(); i++) {
		fileGameItems[i].position.x = fileGameItems[i].initialPositions.x;
		fileGameItems[i].position.y = fileGameItems[i].initialPositions.y;
	}

	for (size_t i = 0; i < fileGameEnemies.size(); i++) {
		fileGameEnemies[i].position.x = fileGameEnemies[i].initialPositions.x;
		fileGameEnemies[i].position.y = fileGameEnemies[i].initialPositions.y;
	}

	gotBanan = false;
	itemGot = false;
	stardead = false;
	gameBoot = true;

	alphaVar = 0.0f;

	indexForSprite = 0;
	itemIndex = 0;
	shinyItemIndex = 4;
	slimeIndex = 0;
	grottoIndex = 4;
}

bool readHeader(std::ifstream &stream) {
	std::string line;
	mapWidth = -1;
	mapHeight = -1;

	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			mapHeight = atoi(value.c_str());
		}
	}

	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { //Allocate our map data
		levelData = new unsigned int*[mapHeight];
		for (int i = 0; i < mapHeight; i++) {
			levelData[i] = new unsigned int[mapWidth];
		}

		return true;
	}

}

void readLayerData(std::ifstream &stream) {
	std::string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				std::istringstream lineStream(line);
				std::string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					unsigned int val = (unsigned int)atoi(tile.c_str());
					if (val > 0) {
						//Tiles in this format are index from 1 not 0
						levelData[y][x] = val - 1;
						//Check our val!

					}
					else {
						levelData[y][x] = 0;
					}

				}
			}
		}
	}

}

void readEntityData(std::ifstream &stream) {
	std::string line;
	std::string type;
	std::vector<Entity> entities;

	while (getline(stream, line)) {
		if (line == "") { break; }
		std::istringstream sStream(line);
		std::string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);

		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			std::istringstream lineStream(value);
			std::string xPos, yPos;
			getline(lineStream, xPos, ',');
			getline(lineStream, yPos, ',');

			float placeX = (float)atoi(xPos.c_str()) * TILE_SIZE;
			float placeY = (float)atoi(yPos.c_str()) * -TILE_SIZE;



			if (type == "EntityPlayer") {
				fileGameHero.setPos(placeX, placeY);
				fileGameHero.initialPositions.x = placeX;
				fileGameHero.initialPositions.y = placeY;
				fileGameHero.checkPointPositions.x = placeX;
				fileGameHero.checkPointPositions.y = placeY;

			}
			else if (type == "EntityEnemyGrotto") {
				Enemy enemy(placeX, placeY);
				enemy.name = "Grotto";
				enemy.initialPositions.x = placeX;
				enemy.initialPositions.y = placeY;
				fileGameEnemies.push_back(enemy);

			}
			else if (type == "EntityEnemySlime") {
				Enemy enemy(placeX, placeY);
				enemy.name = "Slime";
				enemy.initialPositions.x = placeX;
				enemy.initialPositions.y = placeY;
				fileGameEnemies.push_back(enemy);
			}
			else if (type == "EntityEnemyFly") {
				continue; //Decided against these
				/*
				Enemy enemy(placeX, placeY);
				enemy.name = "Fly";
				enemy.initialPositions.x = placeX;
				enemy.initialPositions.y = placeY;
				fileGameEnemies.push_back(enemy);
				*/
			}
			else if (type == "EntityItemWin") {
				Item item(placeX, placeY);
				item.name = "Win";
				item.initialPositions.x = placeX;
				item.initialPositions.y = placeY;
				fileGameItems.push_back(item);
			}
			else if (type == "EntityItemBanana") {
				Item item(placeX, placeY);
				item.name = "Shiny";
				item.initialPositions.x = placeX;
				item.initialPositions.y = placeY;
				fileGameItems.push_back(item);
			}
			else if (type == "EntityItemMusic") {
				Item item(placeX, placeY);
				item.track = globalTrack;
				item.name = "Music";
				item.initialPositions.x = placeX;
				item.initialPositions.y = placeY;
				globalTrack++;
				fileGameItems.push_back(item);
			}
		}
	}
}


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

void inFile(std::string documentName) {
	std::ifstream myFile(documentName);
	std::string line;
	while (getline(myFile, line)) {
		//Handle Line
		if (line == "[header]") {
			if (!readHeader(myFile)) {
				return;
			}
		}
		else if (line == "[layer]") {
			readLayerData(myFile);
		}
		else if (line == "[Object Layer 1]") {
			readEntityData(myFile);
		}
	}
}

void ProcessEvents() {
	if (mode == STATE_MAIN_MENU) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
					mode = STATE_GAME_SCREEN;
					musicNow = Mix_LoadMUS("09 Dire, Dire Docks.mp3");
					Mix_PlayMusic(musicNow, -1);
					Mix_VolumeMusic(20);
					//ResetGame Function
					//program2.SetScreen(1.0f);

				}
			}
		}
	}
		

	else if (mode == STATE_GAME_SCREEN) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				Mix_HaltMusic();
			}

			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && fileGameHero.collidedBottom && itemGot == false) {
					fileGameHero.velocity.y = 3.0f;
					fileGameHero.collidedBottom = false;
					didAJump = true;
					Mix_PlayChannel(-1, jumpMan, 0);
				}

			}
		}

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		//Player movement
		if (keys[SDL_SCANCODE_RIGHT]) {
			//Change acceleration here, velocity changes within Update function of our hero
			//Friction will change in our update function, but only applies to our player!
			if (!itemGot) {
				fileGameHero.acceleration.x = 1.0f;
				playerSwitch = false;
				indexForSprite++;
				if (indexForSprite > 2) {
					indexForSprite = 0;
				}
			}
		}
		else if (keys[SDL_SCANCODE_LEFT]) {
			if (!itemGot) {
				fileGameHero.acceleration.x = -1.0f;
				playerSwitch = true;
				indexForSprite++;
				if (indexForSprite > 2) {
					indexForSprite = 0;
				}
			}
		}
		else {
			fileGameHero.acceleration.x = 0.0f;
			fileGameHero.acceleration.y = 0.0f;

		}

		//Way to quit the game
		if (keys[SDL_SCANCODE_Q]) {
			done = true;
		}
	}
		
		
	else if (mode == STATE_GAME_OVER) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_TAB) {
					mode = STATE_MAIN_MENU;
					//ResetGame Function

				}
			}
		}
	}

}




void Setup() {
	screenwidth = 1280;
	screenheight = 800;
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenwidth, screenheight, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);


#ifdef _WINDOWS
	glewInit();
#endif

	aspectRatio = (float)screenwidth / (float)screenheight; //of screen
	projectionHeight = 2.0f;
	projectionWidth = projectionHeight * aspectRatio;
	projectionDepth = 2.0f;

	glViewport(0, 0, screenwidth, screenheight);
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	//program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
	
	float lastFrameTicks = 0.0f;

	//Blend Here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Get Texture IDS
	playerID = LoadTexture(RESOURCE_FOLDER"player.png");
	enemiesID = LoadTexture(RESOURCE_FOLDER"enemies.png");
	itemsID = LoadTexture(RESOURCE_FOLDER"items.png");
	tilesID = LoadTexture(RESOURCE_FOLDER"beastlands.png");
	txtTexture = LoadTexture(RESOURCE_FOLDER "BlockFont.png");


	//Overall setup
	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-projectionWidth, projectionWidth, -projectionHeight, projectionHeight, -projectionDepth, projectionDepth);

	inFile("MapFinal.txt");

	fileGameHero.textureID = playerID;
	for (size_t i = 0; i < fileGameEnemies.size(); i++) {
		fileGameEnemies[i].textureID = enemiesID;
	}
	for (size_t i = 0; i < fileGameItems.size(); i++) {
		fileGameItems[i].textureID = itemsID;
	}


	//Sounds
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	jumpMan = Mix_LoadWAV("UGH HUH.wav");
	shineGet = Mix_LoadWAV("341695__projectsu012__coins-1.wav");
	musicNow = Mix_LoadMUS("09 Dire, Dire Docks.mp3");
	//Mix_VolumeChunk(jumpMan, 20);
	



}


void Render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	
	program.SetProjectionMatrix(projectionMatrix);
	//program2.SetProjectionMatrix(projectionMatrix);
	
	
	


	if (mode == STATE_GAME_SCREEN) {

		//program2.SetAlpha(0.0f);
		
		viewMatrix = glm::mat4(1.0f);
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-fileGameHero.position.x, -fileGameHero.position.y, 0));
		if (playerIsHit) {
			if (screenShake > 1.0f) {
				playerIsHit = false;
			}
			else {
				viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, sin(screenShake * 25.0) * 0.2, 0.0f));
			}

		}

		

	
		program.SetViewMatrix(viewMatrix);
		//program2.SetViewMatrix(viewMatrix);
		

		drawLevelData(levelData);

		fileGameHero.Render(program);

		
		for (size_t i = 0; i < fileGameEnemies.size(); i++) {
			fileGameEnemies[i].Render(program);
		}
		


		if (!(itemGot)) {
			for (size_t i = 0; i < fileGameItems.size(); i++) {
				fileGameItems[i].Render(program);
			}
		}

		

	}
	else if (mode == STATE_MAIN_MENU) {
		viewMatrix = glm::mat4(1.0f);
		program.SetViewMatrix(viewMatrix);
		//program2.SetViewMatrix(viewMatrix);

		DrawText(program, txtTexture, titleprint, 0.3f, 0.0f, -1.5f, 0.0f);
		DrawText(program, txtTexture, commandprint, 0.1f, 0.0f, -0.6f, -0.4f);

	}
	else if (mode == STATE_GAME_OVER) {
		viewMatrix = glm::mat4(1.0f);
		program.SetViewMatrix(viewMatrix);
		//program2.SetViewMatrix(viewMatrix);

		DrawText(program, txtTexture, gameOver, 0.3f, 0.0f, -3.0f, 0.0f);
		DrawText(program, txtTexture, gameoverCommandPrint, 0.1f, 0.0f, -1.1f, -0.4f);

		if (gotBanan) {
			DrawText(program, txtTexture, secret, 0.1f, 0.0f, -2.0f, -0.8f);
			DrawText(program, txtTexture, secret1, 0.1f, 0.0f, -2.0f, -1.0f);
			DrawText(program, txtTexture, secret2, 0.1f, 0.0f, -2.0f, -1.2f);
			DrawText(program, txtTexture, secret3, 0.1f, 0.0f, -2.0f, -1.4f);
		}

	}
	
	
	


}

void Update(float elapsed) {
	if (mode == STATE_MAIN_MENU) {
		resetGame();
	}
	else if (mode == STATE_GAME_SCREEN) {
		deadTime += elapsed;
		if (deadTime > 8.0f && stardead) {
			gameBoot = false;
		}
		if (deadTime > 10.0f && stardead) {
			mode = STATE_GAME_OVER;
			//alphaVar = 0.0f;
		}


		if (gameBoot == false) {
			alphaVar -= elapsed;
			if (alphaVar < 0.0f) {
				alphaVar = 0.0f;
			}

		}
		else {
			alphaVar += elapsed;
			if (alphaVar > 1.0f) {
				alphaVar = 1.0f;
			}
		}



		fileGameHero.Update(elapsed);

		
		for (size_t i = 0; i < fileGameEnemies.size(); i++) {
			fileGameEnemies[i].Update(elapsed);
		}
		


		for (size_t i = 0; i < fileGameItems.size(); i++) {
			fileGameItems[i].Update(elapsed);
		}

		/*
		animationTime = animationTime + elapsed;
		float animationValue = mapValue(animationTime, 5.0f, 10.0f, 0.0f, 1.0f);
		modelMatrix = glm::mat4(1.0f);
		float xPos = lerp(0.0, 3.0, animationValue);
		glm::translate(modelMatrix, glm::vec3(xPos, 0.0f, 0.0f));
		*/
	}
	else if(mode == STATE_GAME_OVER) {
		//resetGame();
	}
	


}

bool validPosition(int y, int x) {
	if (y >= 0 && x >= 0 && y < mapHeight && x < mapWidth) {
		return true;
	}
	return false;
}

//This is fixed!! Apply to enemy!
void Player::CollisionX() {

	int worldToTilexLeft = (int)((position.x - (TILE_SIZE / 2.0f)) / TILE_SIZE);
	int worldToTilexRight = (int)((position.x + (TILE_SIZE / 2.0f)) / TILE_SIZE);
	int worldToTiley = (int)((position.y) / -TILE_SIZE);
	//Collision!!!!
	if (validPosition(worldToTiley, worldToTilexLeft) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTiley][worldToTilexLeft]) != staticIndex.end()) {
		//Player left hit tiled right?
		float penetration = abs((position.x - TILE_SIZE / 2.0f) - (TILE_SIZE * worldToTilexLeft + TILE_SIZE));
		position.x += (penetration + 0.002f);
		collidedLeft = true;

	}
	else if (validPosition(worldToTiley, worldToTilexRight) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTiley][worldToTilexRight]) != staticIndex.end()) {
		//Player right hit tiled left?
		float penetration = abs((TILE_SIZE * worldToTilexRight) - (position.x + TILE_SIZE / 2.0f));
		position.x -= (penetration + 0.002f);
		collidedRight = true;
	}
}

void Player::CollisionY() {

	int worldToTileyBottom = (int)((position.y - (TILE_SIZE / 2.0f)) / -TILE_SIZE);
	int worldToTileyTop = (int)((position.y + (TILE_SIZE / 2.0f)) / -TILE_SIZE);
	int worldToTilex = (int)((position.x) / TILE_SIZE);

	//Collision with bottom!
	if (validPosition(worldToTileyBottom, worldToTilex) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTileyBottom][worldToTilex]) != staticIndex.end()) {
		//Player bottom hit tiled top?
		float penetration = abs((-TILE_SIZE * worldToTileyBottom) - (position.y - TILE_SIZE / 2.0f));
		position.y += (penetration + 0.002f);
		collidedBottom = true;

		if (didAJump) {
			didAJump = false;
			landedFromAJump = true;
		}

	}
	else if (validPosition(worldToTileyTop, worldToTilex) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTileyTop][worldToTilex]) != staticIndex.end()) {
		//Player top hit tiled bottom?
		float penetration = abs((position.y + TILE_SIZE / 2.0f) - (-TILE_SIZE * worldToTileyTop - TILE_SIZE));
		position.y -= (penetration + 0.002f);
		collidedTop = true;
	}

}


void Player::Update(float elapsed) {
	//Acc
	acceleration.y = -1.0f;

	//Friction
	velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
	velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);

	//Velocity Changes
	velocity.x += acceleration.x * elapsed;
	velocity.y += acceleration.y * elapsed;

	//Reset
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;
	collidedTop = false;

	//Position
	position.y += velocity.y * elapsed;
	CollisionY();

	position.x += velocity.x * elapsed;
	CollisionX();




	//Item collisions and what happens! Checkpoint and music change!!!
	for (size_t i = 0; i < fileGameItems.size(); i++) {
		float p1 = abs(position.x - fileGameItems[i].position.x) - ((TILE_SIZE + TILE_SIZE) / 2.0f);
		float p2 = abs(position.y - fileGameItems[i].position.y) - ((TILE_SIZE + TILE_SIZE) / 2.0f);
		if (p1 < 0 && p2 < 0) {
			if (fileGameItems[i].name == "Win") {
				itemGot = true;
				fileGameItems[i].position.x = -500.0f;
				fileGameItems[i].position.y = -500.0f;
				musicNow = Mix_LoadMUS("25 Course Clear.mp3");
				Mix_PlayMusic(musicNow, 1);
				Mix_VolumeMusic(40);

				//Wait 10 seconds for everything to settle
				
				stardead = true;
				deadTime = 0.0f;


			}
			else if (fileGameItems[i].name == "Shiny") {
				gotBanan = true;
				Mix_PlayChannel(-1, shineGet, 0);
				fileGameItems[i].position.x = -500.0f;
				fileGameItems[i].position.y = -500.0f;
			}
			else if (fileGameItems[i].name == "Music") {
				if (fileGameItems[i].track == 0) {
					musicNow = Mix_LoadMUS("05 Cave Dweller Concert.mp3");
					//Mix_VolumeChunk(jumpMan, 20);
					Mix_PlayMusic(musicNow, -1);
					Mix_VolumeMusic(20);
					fileGameItems[i].position.x = -500.0f;
					fileGameItems[i].position.y = -500.0f;
					checkPointPositions.x = position.x;
					checkPointPositions.y = position.y;
				}
				else if (fileGameItems[i].track == 1) {
					musicNow = Mix_LoadMUS("10 Lethal Lava Land.mp3");
					//Mix_VolumeChunk(jumpMan, 20);
					Mix_PlayMusic(musicNow, -1);
					Mix_VolumeMusic(20);

					fileGameItems[i].position.x = -500.0f;
					fileGameItems[i].position.y = -500.0f;
					checkPointPositions.x = position.x;
					checkPointPositions.y = position.y;
				}
			}
			
		}
	}

	//Enemy Collision, on collision just get sent back to last checkpoint!
	for (size_t i = 0; i < fileGameEnemies.size(); i++) {
		float p1 = abs(position.x - fileGameEnemies[i].position.x) - ((TILE_SIZE + TILE_SIZE) / 2.0f);
		float p2 = abs(position.y - fileGameEnemies[i].position.y) - ((TILE_SIZE + TILE_SIZE) / 2.0f);
		if (p1 < 0 && p2 < 0) {
			position.x = checkPointPositions.x;
			position.y = checkPointPositions.y;
			Mix_PlayChannel(-1, jumpMan, 0);
			playerIsHit = true;
			screenShake = 0.0f;
			
		}
	}
	
	screenShake += elapsed;

}

void Enemy::CollisionX() {

	int worldToTilexLeft = (int)((position.x - (TILE_SIZE / 2.0f)) / TILE_SIZE);
	int worldToTilexRight = (int)((position.x + (TILE_SIZE / 2.0f)) / TILE_SIZE);
	int worldToTiley = (int)((position.y) / -TILE_SIZE);
	//Collision!!!!
	if (validPosition(worldToTiley, worldToTilexLeft) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTiley][worldToTilexLeft]) != staticIndex.end()) {
		//Player left hit tiled right?
		float penetration = abs((position.x - TILE_SIZE / 2.0f) - (TILE_SIZE * worldToTilexLeft + TILE_SIZE));
		position.x += (penetration + 0.002f);
		collidedLeft = true;

	}
	else if (validPosition(worldToTiley, worldToTilexRight) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTiley][worldToTilexRight]) != staticIndex.end()) {
		//Player right hit tiled left?
		float penetration = abs((TILE_SIZE * worldToTilexRight) - (position.x + TILE_SIZE / 2.0f));
		position.x -= (penetration + 0.002f);
		collidedRight = true;
	}


}

void Enemy::CollisionY() {

	int worldToTileyBottom = (int)((position.y - (TILE_SIZE / 2.0f)) / -TILE_SIZE);
	int worldToTileyTop = (int)((position.y + (TILE_SIZE / 2.0f)) / -TILE_SIZE);
	int worldToTilex = (int)((position.x) / TILE_SIZE);

	//Collision with bottom!
	if (validPosition(worldToTileyBottom, worldToTilex) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTileyBottom][worldToTilex]) != staticIndex.end()) {
		//Player bottom hit tiled top?
		float penetration = abs((-TILE_SIZE * worldToTileyBottom) - (position.y - TILE_SIZE / 2.0f));
		position.y += (penetration + 0.002f);
		collidedBottom = true;

	}
	else if (validPosition(worldToTileyTop, worldToTilex) && std::find(staticIndex.begin(), staticIndex.end(), levelData[worldToTileyTop][worldToTilex]) != staticIndex.end()) {
		//Player top hit tiled bottom?
		float penetration = abs((position.y + TILE_SIZE / 2.0f) - (-TILE_SIZE * worldToTileyTop - TILE_SIZE));
		position.y -= (penetration + 0.002f);
		collidedTop = true;
	}

	//Turn around slime
	if (validPosition(worldToTileyBottom, worldToTilex) && std::find(turnAroundSlime.begin(), turnAroundSlime.end(), levelData[worldToTileyBottom][worldToTilex]) != turnAroundSlime.end()) {
		if (levelData[worldToTileyBottom][worldToTilex] == 863) {
			acceleration.x = 0.6;
		}
		else if (levelData[worldToTileyBottom][worldToTilex] == 862) {
			acceleration.x = -0.6;
		}
	}

}


void Enemy::Update(float elapsed) {


	if (name == "Slime") {
		timerEnemy += elapsed;
		if (timerEnemy > 0.05f) {
			slimeIndex++;
			timerEnemy = 0;
		}
		if (slimeIndex > 2) {
			slimeIndex = 0;
		}
	}

	else if (name == "Grotto") {
		timerEnemy += elapsed;
		if (timerEnemy > 0.05f) {
			grottoIndex++;
			timerEnemy = 0;
		}
		if (grottoIndex > 7) {
			grottoIndex = 4;
		}
	}





	//Acc
	acceleration.y = -1.0f;

	//Friction --> Might remove to keep movement consistent
	
	velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
	velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);
	
	//Velocity Changes
    velocity.x += acceleration.x * elapsed;
	velocity.y += acceleration.y * elapsed;

	//Reset
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;
	collidedTop = false;

	//Position
	position.y += velocity.y * elapsed;
	CollisionY();

	position.x += velocity.x * elapsed;

	CollisionX();
	if (collidedRight == true) {
		acceleration.x = -0.6f;
	}
	else if (collidedLeft == true) {
		acceleration.x = 0.6f;
	}

	//2174 is the first tile and 2180 --> Hardcoded edge positions for slime on the floating platform!

}

void Item::Update(float elapsed) {

	//Hover
	hover += elapsed;
	
	if (name == "Win") {
		timerItem += elapsed;
		if (timerItem > 0.05f) {
			itemIndex++;
			timerItem = 0;
		}
		if (itemIndex > 2) {
			itemIndex = 0;
		}
	}
	
	else if (name == "Shiny") {
		shinytimerItem += elapsed;
		if (shinytimerItem > 0.05f) {
			shinyItemIndex++;
			shinytimerItem = 0;
		}
		if (shinyItemIndex > 7) {
			shinyItemIndex = 4;
		}
	}
	
	

}

void drawLevelData(unsigned int** levelData) {
	glBindTexture(GL_TEXTURE_2D, tilesID);
	std::vector<float> vertexData;
	//vertexData.size() / 2
	std::vector<float> texCoordData;

	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X_TILE) / (float)SPRITE_COUNT_X_TILE;
			float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X_TILE) / (float)SPRITE_COUNT_Y_TILE;

			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X_TILE;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y_TILE;

			vertexData.insert(vertexData.end(), {
				TILE_SIZE * x, -TILE_SIZE * y,
				TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,

				TILE_SIZE * x, -TILE_SIZE * y,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y



				});

			texCoordData.insert(texCoordData.end(), {
				u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),

				u,v,
				u + spriteWidth, v + (spriteHeight),
				u + spriteWidth, v
				});
		}
	}

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	program.SetModelMatrix(modelMatrix);
	//program2.SetModelMatrix(modelMatrix);

	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

int min(float f1, float f2) {
	if (f1 > f2) return f2;
	return f1;
}

int max(float f1, float f2) {
	if (f1 > f2) return f1;
	return f2;
}

void Player::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	if (!(collidedBottom)) {
		indexForSprite = 4;
	}
	else if (landedFromAJump) {
		indexForSprite = 6;
		landedFromAJump = false;
	}

	if (itemGot) {
		indexForSprite = 7;
	}
	float u = (float)(((int)indexForSprite) % SPRITE_COUNT_X_PLAYER) / (float)SPRITE_COUNT_X_PLAYER;
	float v = (float)(((int)indexForSprite) / SPRITE_COUNT_X_PLAYER) / (float)SPRITE_COUNT_Y_PLAYER;

	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X_PLAYER;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y_PLAYER;


	vertexData.insert(vertexData.end(), {
		-0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,

		0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f
		});

	texCoordData.insert(texCoordData.end(), {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,

		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
		});

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	//viewMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE, TILE_SIZE, 1));

	//Stretch and squash
	/*
	if (collidedBottom == false) {
		float scale_y = mapValue(fabs(velocity.y), 0.0, 10.0, 1.0, 1.6);
		float scale_x = mapValue(fabs(velocity.y), 10.0, 0.0, 0.8, 1.0);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale_x, scale_y, 1.0f));
	}
	*/

	if (playerSwitch) {
		modelMatrix = glm::scale(modelMatrix, glm::vec3(-1.0f, 1.0f, 1.0f));
	}
	

	//X, min(max(position.x, level width), -projection width)
	//Y, min(max(position.y, level height), projection height)
	//Make sure they are all negative (except projection height)
	//viewMatrix = glm::translate(viewMatrix, glm::vec3(max(-position.x, -(TILE_SIZE * 128.0f)) , -position.y, 0));
	//Fix the above soon

	

	program.SetModelMatrix(modelMatrix);
	//program2.SetModelMatrix(modelMatrix);
	//program.SetViewMatrix(viewMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void Enemy::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float u;
	float v;

	if (name == "Grotto") {
		u = (float)(((int)grottoIndex) % SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_X_ENEMY;
		v = (float)(((int)grottoIndex) / SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_Y_ENEMY;
	}
	else if (name == "Slime") {
		u = (float)(((int)slimeIndex) % SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_X_ENEMY;
		v = (float)(((int)slimeIndex) / SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_Y_ENEMY;
	}
	

	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X_ENEMY;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y_ENEMY;


	vertexData.insert(vertexData.end(), {

		-0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,

		0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f

		});

	texCoordData.insert(texCoordData.end(), {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,

		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
		});

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);

	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE, TILE_SIZE, 1));
	if (acceleration.x < 0.0f) {
		modelMatrix = glm::scale(modelMatrix, glm::vec3(-1, 1, 1));
	}

	program.SetModelMatrix(modelMatrix);
	//program2.SetModelMatrix(modelMatrix);




	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);



}


void Item::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float u;
	float v;

	if (name == "Win") {
		u = (float)(((int)itemIndex) % SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_X_ITEM;
		v = (float)(((int)itemIndex) / SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_Y_ITEM;
	}
	else if (name == "Music") {
		u = (float)(((int)3) % SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_X_ITEM;
		v = (float)(((int)3) / SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_Y_ITEM;
	}
	else if (name == "Shiny") {
		u = (float)(((int)shinyItemIndex) % SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_X_ITEM;
		v = (float)(((int)shinyItemIndex) / SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_Y_ITEM;
	}


	float spriteWidth = 1.0f / (float)SPRITE_COUNT_X_ITEM;
	float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y_ITEM;

	vertexData.insert(vertexData.end(), {

		-0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,

		0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f

		});

	texCoordData.insert(texCoordData.end(), {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,

		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
		});

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Modify modelView here
	modelMatrix = glm::mat4(1.0f);
	
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE, TILE_SIZE, 1));

	

	program.SetModelMatrix(modelMatrix);
	//program2.SetModelMatrix(modelMatrix);
	

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);


}



int main(int argc, char *argv[])
{

	//Check to see if git push

	//Make a new shader, use it until our alpha thing is 0 and switch back to the old one!!
	Setup();

	while (!done) {
		ProcessEvents();


		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		elapsed += accumulator;

		if (elapsed < FIXED_TIMESTEP) {
			accumulator = elapsed;
			continue;
		}
		while (elapsed >= FIXED_TIMESTEP) {
			Update(FIXED_TIMESTEP);
			elapsed -= FIXED_TIMESTEP;
		}
		accumulator = elapsed;


		Render();


		SDL_GL_SwapWindow(displayWindow);
	}

	Mix_FreeChunk(jumpMan);
	Mix_FreeChunk(shineGet);
	Mix_FreeMusic(musicNow);
	SDL_Quit();

	return 0;
}
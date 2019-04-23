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


#define FIXED_TIMESTEP 0.016666666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 0.3f
#define SPRITE_COUNT_X_TILE 59
#define SPRITE_COUNT_Y_TILE 34

#define SPRITE_COUNT_X_ITEM 4
#define SPRITE_COUNT_Y_ITEM 4

#define SPRITE_COUNT_X_ENEMY 4
#define SPRITE_COUNT_Y_ENEMY 3

#define SPRITE_COUNT_X_PLAYER 4
#define SPRITE_COUNT_Y_PLAYER 2


#include "ShaderProgram.h"
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
SDL_Window* displayWindow;

//FileReading.h


int mapWidth;
int mapHeight;
unsigned int** levelData;

bool readHeader(std::ifstream &stream);
void readLayerData(std::ifstream &stream);
void readEntityData(std::ifstream &stream);


//End


//Entity.h
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
		friction.x = 0.2f;
		friction.y = 0.2f;
	}
	Player() : Entity() {}
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
	}
	virtual void Render(ShaderProgram &program);
	virtual void Update(float elapsed);
	//virtual bool CollidesWith(Entity &entity);
};

class Enemy : public Entity {
public:
	Enemy(float posX, float posY) : Entity(posX, posY) {}
	Enemy() : Entity() {}
	std::string name;
	virtual void Render(ShaderProgram &program);
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
	}
	//virtual void Update(float elapsed);
	//virtual bool CollidesWith(Entity &entity);

};

class Item : public Entity {
public:
	Item(float posX, float posY) : Entity(posX, posY) {}
	Item() : Entity() {}
	virtual void Render(ShaderProgram &program);
	virtual void setPos(float x, float y) {
		position.x = x;
		position.y = y;
	}
	//virtual bool CollidesWith(Entity &entity);

};

float lerp(float v0, float v1, float t);


//End


bool done = false;
SDL_Event event;
//std::vector<std::vector<MapTile>> fileMapTiles;
Player fileGameHero;
std::vector<Enemy> fileGameEnemies;
std::vector<Item> fileGameItems;

int screenwidth;
int screenheight;
float aspectRatio;
float projectionWidth;
float projectionHeight;
float projectionDepth;

float elapsed;
float ticks;
float lastFrameTicks;
float accumulator;
float acceleration_x = 0.2f;
float acceleration_y = 0.5f;

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

glm::mat4 modelMatrix;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

std::vector<int> staticIndex = { 355, 479, 480, 538, 539, 356, 357, 358, 414, 415, 416, 417, 473, 474, 475, 476, 532, 533, 534, 535, 424, 425, 426, 542, 543, 544, 496, 497, 498, 797, 576, 577, 798, 799, 800, 801, 802, 856, 857, 858, 859, 860, 861, 915, 916, 917, 483, 484, 485 };

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
		//std::vector<std::vector<MapTile>> newFileMap(mapHeight, std::vector<MapTile>(mapWidth));
		//fileMapTiles = newFileMap;
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
				
			}
			else if (type == "EntityEnemy") {
				Enemy enemy(placeX, placeY);
				fileGameEnemies.push_back(enemy);

			}
			else if (type == "EntityItem") {
				Item item(placeX, placeY);
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
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			done = true;
		}
	}

	
	

	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	//Player movement
	if (keys[SDL_SCANCODE_RIGHT]) {
		//Change acceleration here, velocity changes within Update function of our hero
		//Friction will change in our update function, but only applies to our player!
		if (fileGameHero.velocity.x <= 5.0f) {
			fileGameHero.velocity.x += acceleration_x * elapsed;
		}
		
	}
	else if (keys[SDL_SCANCODE_LEFT]) {
		if (fileGameHero.velocity.x >= -5.0f) {
			fileGameHero.velocity.x -= acceleration_x * elapsed;
		}
		
	}
}

/*
void drawMapSetup(char location, int i, int y) {
	MapTile tile((float)i, (float)y);
	int locationint = location - '0';
	if (std::find(staticIndex.begin(), staticIndex.end(), locationint) != staticIndex.end()) {
		tile.isStatic = true;
	}
	tile.textureID = tilesID;
	tile.leveli = i;
	tile.levely = y;
	fileMapTiles[y][i] = tile;
}
*/



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
	float lastFrameTicks = 0.0f;

	//Blend Here
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Get Texture IDS
	playerID = LoadTexture(RESOURCE_FOLDER"player.png");
	enemiesID = LoadTexture(RESOURCE_FOLDER"enemies.png");
	itemsID = LoadTexture(RESOURCE_FOLDER"items.png");
	tilesID = LoadTexture(RESOURCE_FOLDER"beastlands.png");


	//Overall setup
	projectionMatrix = glm::mat4(1.0f);
	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-projectionWidth, projectionWidth, -projectionHeight, projectionHeight, -projectionDepth, projectionDepth);
	 
	inFile("Map1.txt");

	fileGameHero.textureID = playerID;
	for (size_t i = 0; i < fileGameEnemies.size(); i++) {
		fileGameEnemies[i].textureID = enemiesID;
	}
	for (size_t i = 0; i < fileGameItems.size(); i++) {
		fileGameItems[i].textureID = itemsID;
	}

	/*
	for (int y = 0; y < mapHeight; y++) {
		for (int i = 0;  i < mapWidth; i++) {
			drawMapSetup(levelData[i][y], i, y);
		}
	}
	*/

}


void Render() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);


	drawLevelData(levelData);

	fileGameHero.Render(program);
	for (size_t i = 0; i < fileGameEnemies.size(); i++) {
		fileGameEnemies[i].Render(program);
	}
	for (size_t i = 0; i < fileGameItems.size(); i++) {
		fileGameItems[i].Render(program);
	}

	/*
	for (size_t y = 0; y < fileMapTiles.size(); y++) {
		for (size_t i = 0; i < fileMapTiles[y].size(); i++) {
			fileMapTiles[y][i].Render(program);
		}
	}
	*/

	
}

float lerp(float v0, float v1, float t) {
	return (1.0f - t)*v0 + t * v1;
}


void Player::Update(float elapsed) {
	//Friction
	velocity.x = lerp(velocity.x, 0.0f, elapsed * friction.x);
	velocity.y = lerp(velocity.y, 0.0f, elapsed * friction.y);

	//Position
	position.y += velocity.y * elapsed;
	//CollisionY();
	position.x += velocity.x * elapsed;
	//CollisionX();




	//Check for collision!
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
				TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,

				TILE_SIZE * x, -TILE_SIZE * y,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
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

	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

}

int min(float f1, float f2) {
	if (f1 > f2) return f2;
	return f1;
}

void Player::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float u = (float)(((int)1) % SPRITE_COUNT_X_PLAYER) / (float)SPRITE_COUNT_X_PLAYER;
	float v = (float)(((int)1) / SPRITE_COUNT_X_PLAYER) / (float)SPRITE_COUNT_Y_PLAYER;

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
	viewMatrix = glm::mat4(1.0f);
	//Translate, Rotate, Scale
	modelMatrix = glm::translate(modelMatrix, glm::vec3(position.x, position.y, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(TILE_SIZE, TILE_SIZE, 1));

	viewMatrix = glm::translate(viewMatrix, glm::vec3(min(-position.x, projectionWidth), min(-position.y, projectionHeight), 0));


	program.SetModelMatrix(modelMatrix);
	program.SetViewMatrix(viewMatrix);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void Enemy::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float u = (float)(((int)0) % SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_X_ENEMY;
	float v = (float)(((int)0) / SPRITE_COUNT_X_ENEMY) / (float)SPRITE_COUNT_Y_ENEMY;

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
		u, v,
		u, v + (spriteHeight),
		u + spriteWidth, v + (spriteHeight),

		u,v,
		u + spriteWidth, v + (spriteHeight),
		u + spriteWidth, v
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

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);



}


void Item::Render(ShaderProgram &program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float u = (float)(((int)2) % SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_X_ITEM;
	float v = (float)(((int)2) / SPRITE_COUNT_X_ITEM) / (float)SPRITE_COUNT_Y_ITEM;

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
		u, v,
		u, v + (spriteHeight),
		u + spriteWidth, v + (spriteHeight),

		u,v,
		u + spriteWidth, v + (spriteHeight),
		u + spriteWidth, v
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

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);


}


int main(int argc, char *argv[])
{


	Setup();

	while (!done) {
		ProcessEvents();

		/*
		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
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
		*/

		Render();

		
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
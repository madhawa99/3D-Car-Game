#include "Car.h"
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <Gl/glut.h>
#include "Car.h"
#include <chrono>
#include <stdio.h>
#include <SOIL2.h>

#define ESC 27
using namespace std;
constexpr float PI = 3.14159265358979323846;


string projectPath = "F:\\Graphic\\project 3d game\\"; 
string texturesPath = projectPath + "textures\\";

int screenWidth, screenHeight;
float aspect;
float carX = 0.15, carZ = 1, carSpeed = 1.0;
float cameraX, cameraY, cameraZ;
float roadRight = 2, roadLeft = -2.5, roadStart = 7, roadEnd = -500;
int mouselastX, mouselastY;
bool buffer[256];
unsigned int textures[10];
int collectedCoins;
bool win, gameover;
float coinAngle;

struct Coin {
	float x, z;
	bool colided = false, special = false;
};

struct Obstacle {
	float x1, x2, z;
};

vector<Coin>coins;
vector<Obstacle>obstacles;

void delay(float sec) {
	auto start = chrono::steady_clock::now();
	chrono::milliseconds duration((int)(1000 * sec));
	while (chrono::steady_clock::now() - start < duration);
}


bool loadTexture(const string& filename, int textureIndex) {
	int width, height, channels;

	
	unsigned char* image = SOIL_load_image((texturesPath + filename).c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image) {
		cout << "Failed to load texture: " << filename << endl;
		cout << "SOIL error: " << SOIL_last_result() << endl;
		return false;
	}

	// Generate and bind the texture
	glGenTextures(1, &textures[textureIndex]);
	glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SOIL_free_image_data(image);

	return true;
}





// Coins  & Obstacles handling
float random(float min = 0, float max = RAND_MAX) {
	if (min > max) swap(max, min);
	return min + float(rand()) / (float(RAND_MAX / (max - min)));
}

void setCoins(int n = 20) {
	if (n < 20) { n = 20; }
	int special = 10; n -= special;
	while (n--) {
		coins.push_back({ random(roadLeft + 0.2, roadRight - 0.2), random(roadStart - 5, roadEnd + 5) });
	}
	while (special--) {
		coins.push_back({ random(roadLeft + 0.2, roadRight - 0.2), random(roadStart - 5, roadEnd + 5), false, true });
	}
}

void setObstacles(int n = 20) {
	while (n--) {
		float x1 = random(roadLeft + 0.1, roadRight - 0.1), x2;
		x2 = x1 + 1;
		if (x2 > roadRight) {
			x2 -= 2;
		}
		if (x1 > x2) {
			swap(x1, x2);
		}
		obstacles.push_back({ x1, x2, random(roadStart - 10, roadEnd + 5) });
	}
}



// Collision handling
float calculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) {
	return sqrtf(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

bool intersect(float a1, float b1, float a2, float b2) {
/*
	a1---------------b1
			  a2---------b2
*/


	if (a1 > b2 || a2 > b1) {
		return false;
	}
	else {
		return true;
	}
}

int coinsCollision() {
	for (int i = 0; i < coins.size(); i++) {
		if (!coins[i].colided) {
			float coinCenterX = coins[i].x, coinCenterZ = coins[i].z, coinRadius = 0.13;
			float X[2] = { 0, 0.4 };
			for (int j = 0; j < 2; j++) {
				float distance = calculateDistance(carX - X[j], 0, carZ, coinCenterX, 0, coinCenterZ);
				float distance2 = calculateDistance(carX - X[j], 0, carZ + 0.4, coinCenterX, 0, coinCenterZ);
				float distance3 = calculateDistance(carX - X[j], 0, carZ + 0.63, coinCenterX, 0, coinCenterZ);

				if (distance <= X[j] + coinRadius - 0.07 || distance2 <= X[j] + coinRadius - 0.06 || distance3 <= X[j] + coinRadius - 0.05) {

					coins[i].colided = true;
					return 1 + coins[i].special * 9;
				}
			}
		}
	}
	return 0;
}

bool obstaclesCollision() {
	for (int i = 0; i < obstacles.size(); i++) {
		float obstacleRight = obstacles[i].x1, obstacleLeft = obstacles[i].x2, obstacleZ = obstacles[i].z;
		float carRight = carX, carLeft = carX - 0.54;

		if (obstacleLeft > obstacleRight) {
			swap(obstacleLeft, obstacleRight);
		}
		obstacleZ -= 0.3;
		if (abs(obstacleZ - carZ) <= 0.80001 && intersect(obstacleLeft, obstacleRight, carLeft, carRight)) {
			return true;
		}
	}
	return false;
}



// Drawing

///////////////////////////////////////////////////////////////////////////////////////////////

void drawBuilding1() {
	// Main building structure
	glColor3f(0.4, 0.4, 0.4); 
	glPushMatrix();
	glTranslatef(0.0, 2.0, 0.0);
	glScalef(2.0, 4.0, 2.0); 
	glutSolidCube(1.0); 
	glPopMatrix();

	// Windows
	glColor3f(0.1, 0.1, 0.5); 
	float windowSpacing = 0.5;
	float windowSize = 0.2;
	for (float y = 0.5; y < 4.0; y += windowSpacing) {
		for (float x = -0.8; x <= 0.8; x += windowSpacing) {
			glPushMatrix();
			glTranslatef(x, y, 1.01); 
			glScalef(windowSize, windowSize * 1.5, 0.01); 
			glutSolidCube(1.0); 
			glPopMatrix();
		}
	}

	// Rooftop
	glColor3f(0.3, 0.2, 0.1); 
	glPushMatrix();
	glTranslatef(0.0, 4.0, 0.0); 
	glScalef(2.2, 0.2, 2.2); 
	glutSolidCube(1.0); 
	glPopMatrix();

	// Rooftop fence
	glColor3f(0.5, 0.5, 0.5); 
	float fenceSpacing = 0.2;
	for (float x = -1.0; x <= 1.0; x += fenceSpacing) {
		glPushMatrix();
		glTranslatef(x, 4.1, 1.0); 
		glScalef(0.05, 0.3, 0.05); 
		glutSolidCube(1.0); 
		glPopMatrix();
	}

	// Decorative patterns on the building
	glColor3f(0.6, 0.6, 0.6); 
	for (float y = 0.6; y < 4.0; y += 0.6) {
		glPushMatrix();
		glTranslatef(0.0, y, 1.01); 
		glScalef(1.8, 0.1, 0.01); 
		glutSolidCube(1.0); 
		glPopMatrix();
	}

	// Entrance door
	glColor3f(0.2, 0.1, 0.05); 
	glPushMatrix();
	glTranslatef(0.0, 0.5, 1.01); 
	glScalef(0.5, 0.6, 0.01); 
	glutSolidCube(1.0); 
	glPopMatrix();

}

void drawBuilding2() {
	float buildingWidth = 1.5f;  
	float buildingHeight = 10.0f; 
	float windowSpacing = 0.8f;
	float windowSize = 0.3f;
	float leanAngle = 30.0f;

	// Vertical Building (Tower 1)
	glColor3f(0.6, 0.6, 0.6); 
	glPushMatrix();
	glTranslatef(-3.0, 0.0, 0.0); 
	glScalef(buildingWidth, buildingHeight, buildingWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	// Leaning Building (Tower 2)
	glPushMatrix();
	glTranslatef(0.8, -0.5, 0.0); 
	glRotatef(leanAngle, 0.0, 0.0, 1.0);
	glScalef(buildingWidth, buildingHeight, buildingWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	// Windows for Vertical Building (4 sides)
	glColor3f(0.2, 0.4, 0.8); // Glass blue color 
	for (float y = 1.0; y < buildingHeight; y += windowSpacing) {
		for (int side = 0; side < 4; side++) {
			glPushMatrix();
			glTranslatef(-3.0, y - buildingHeight / 2, 0.0);

			// Position windows on all four sides
			switch (side) {
			case 0: glTranslatef(0.76, 0, 0.76); break;  
			case 1: glTranslatef(-0.76, 0, 0.76); break; 
			case 2: glTranslatef(0.76, 0, -0.76); break; 
			case 3: glTranslatef(-0.76, 0, -0.76); break;
			}

			glScalef(windowSize, windowSize * 1.5, windowSize);
			glutSolidCube(1.0);
			glPopMatrix();
		}
	}


	// Windows for Leaning Building (4 sides)
	for (float y = 1.0; y < buildingHeight - 1; y += windowSpacing) {
		for (int side = 0; side < 4; side++) {
			glPushMatrix();
			glTranslatef(0.5, 0.0, 0.0);
			glRotatef(leanAngle, 0.0, 0.0, 1.0);
			glTranslatef(0.0, y - buildingHeight / 2, 0.0);

			// Calculate rotated positions
			float rotX = 0.76 * cos(leanAngle * PI / 180);
			float rotY = 0.76 * sin(leanAngle * PI / 180);

			switch (side) {
			case 0: glTranslatef(rotX, rotY, 0.76); break;
			case 1: glTranslatef(-rotX, -rotY, 0.76); break;
			case 2: glTranslatef(rotX, rotY, -0.76); break;
			case 3: glTranslatef(-rotX, -rotY, -0.76); break;
			}

			glScalef(windowSize, windowSize * 1.5, windowSize);
			glutSolidCube(1.0);
			glPopMatrix();
		}
	}

}




void drawBranch(float length, float radius, int depth) {
	if (depth == 0) return;

	GLUquadric* quad = gluNewQuadric();
	gluCylinder(quad, radius, radius * 0.7, length, 10, 10);
	glTranslatef(0, 0, length);

	if (depth == 1) {
		glColor3f(0.0, 0.6, 0.0);
		glutSolidSphere(radius * 1.5, 10, 10);
	}

	glPushMatrix();
	glRotatef(30, 1, 0, 0);
	drawBranch(length * 0.7, radius * 0.7, depth - 1);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-30, 1, 0, 0);
	drawBranch(length * 0.7, radius * 0.7, depth - 1);
	glPopMatrix();

	glPushMatrix();
	glRotatef(30, 0, 1, 0);
	drawBranch(length * 0.7, radius * 0.7, depth - 1);
	glPopMatrix();

	glPushMatrix();
	glRotatef(-30, 0, 1, 0);
	drawBranch(length * 0.7, radius * 0.7, depth - 1);
	glPopMatrix();

	gluDeleteQuadric(quad);
}

void drawTree1() {
	glPushMatrix();
	glColor3f(0.55, 0.27, 0.07);
	drawBranch(2.0, 0.2, 5);
	glPopMatrix();
}


void drawTree2() {
	GLUquadric* quadric = gluNewQuadric();

	glColor3f(0.5, 0.35, 0.05);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.0);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	gluCylinder(quadric, 0.2, 0.2, 2.0, 10, 10);
	glPopMatrix();


	glColor3f(0.0, 0.8, 0.0);
	float leafSize = 0.8;

	glPushMatrix();
	glTranslatef(-0.4, 1.5, 0.0);
	glutSolidSphere(leafSize, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.4, 1.5, 0.0);
	glutSolidSphere(leafSize, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-0.4, 0.5, 0.0);
	glutSolidSphere(leafSize, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.4, 0.5, 0.0);
	glutSolidSphere(leafSize, 10, 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.5, 0.0);
	glutSolidSphere(leafSize * 1.2, 10, 10);
	glPopMatrix();

	gluDeleteQuadric(quadric);
}



void drawSmallBuilding() {
	// Main building structure
	glColor3f(0.6f, 0.6f, 0.6f); // Gray color
	glutSolidCube(2.0f); // Main cube (2x2x2 units)

	// Corrected pyramid roof
	glColor3f(0.4f, 0.2f, 0.1f);
	glPushMatrix();
	glTranslatef(0.0f, 1.0f, 0.0f);
	glScalef(2, 2, 2);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCone(1.0f, 1.2f, 4, 4);
	glPopMatrix();

	// Door
	glColor3f(0.3f, 0.2f, 0.1f); // Dark brown
	glPushMatrix();
	glTranslatef(0.0f, -0.7f, 1.01f); // Centered on front face
	glScalef(0.4f, 0.6f, 0.1f);
	glutSolidCube(1.0f);
	glPopMatrix();

	// Windows
	glColor3f(0.1f, 0.3f, 0.8f); // Blue windows
	for (int i = -1; i <= 1; i += 2) {
		for (int j = -1; j <= 1; j += 2) {
			glPushMatrix();
			glTranslatef(i * 0.7f, j * 0.4f + 0.2f, 1.01f);
			glScalef(0.3f, 0.3f, 0.1f);
			glutSolidCube(1.0f);
			glPopMatrix();
		}
	}
}





void drawFence() {
	float fenceHeight = 0.5f; 
	float fenceSpacing = 1.0f; 
	float fenceThickness = 0.05f; 

	glColor3f(1.0f, 1.0f, 1.0f); 

	
	for (float z = roadStart; z > roadEnd; z -= fenceSpacing) {
		// Left side of the road
		glPushMatrix();
		glTranslatef(roadLeft, fenceHeight / 2, z);
		glScalef(fenceThickness, fenceHeight, fenceThickness);
		glutSolidCube(1.0f); 
		glPopMatrix();

		// Right side of the road
		glPushMatrix();
		glTranslatef(roadRight, fenceHeight / 2, z);
		glScalef(fenceThickness, fenceHeight, fenceThickness);
		glutSolidCube(1.0f); 
		glPopMatrix();
	}

	glBegin(GL_QUADS);
	for (float z = roadStart; z > roadEnd; z -= fenceSpacing) {
		// Left side rail
		glVertex3f(roadLeft, fenceHeight * 0.8f, z);
		glVertex3f(roadLeft, fenceHeight * 0.8f, z - fenceSpacing);
		glVertex3f(roadLeft, fenceHeight * 0.2f, z - fenceSpacing);
		glVertex3f(roadLeft, fenceHeight * 0.2f, z);

		// Right side rail
		glVertex3f(roadRight, fenceHeight * 0.8f, z);
		glVertex3f(roadRight, fenceHeight * 0.8f, z - fenceSpacing);
		glVertex3f(roadRight, fenceHeight * 0.2f, z - fenceSpacing);
		glVertex3f(roadRight, fenceHeight * 0.2f, z);
	}
	glEnd();

	
}


/*
GLUquadric* quad = gluNewQuadric();

glPushMatrix();
glTranslatef(roadRight, 0, z);
glRotated(-25, 1, 0, 0);
gluCylinder(quad, fenceThickness - 0.01, fenceThickness - 0.01, 1.1, 100, 100);
glPopMatrix();



gluDeleteQuadric(quad);


*/




//Lotus Tower 

// Function to draw a cylinder
void drawCylinder(float baseRadius, float topRadius, float height, int slices, int stacks) {
	GLUquadric* quad = gluNewQuadric();
	gluCylinder(quad, baseRadius, topRadius, height, slices, stacks);
	gluDeleteQuadric(quad);
}

// Function to draw a disk
void drawDisk(float innerRadius, float outerRadius, int slices, int loops) {
	GLUquadric* quad = gluNewQuadric();
	gluDisk(quad, innerRadius, outerRadius, slices, loops);
	gluDeleteQuadric(quad);
}

// Function to draw a sphere
void drawSphere(float radius, int slices, int stacks) {
	GLUquadric* quad = gluNewQuadric();
	gluSphere(quad, radius, slices, stacks);
	gluDeleteQuadric(quad);
}

void drawCylinderWithTexture(float baseRadius, float topRadius, float height, int slices, int stacks) {
	// Draw the solid cylinder
	drawCylinder(baseRadius, topRadius, height, slices, stacks);

	// Add vertical lines for texture
	glColor3f(0.4, 0.2, 0.1); // Darker brown for lines
	for (int i = 0; i < slices; ++i) {
		float angle = 2.0f * PI * i / slices;
		float x = cos(angle) * baseRadius;
		float y = sin(angle) * baseRadius;

		glBegin(GL_LINES);
		glVertex3f(x, y, 0.0);          
		glVertex3f(x, y, height);        
		glEnd();
	}
}


void drawTowerBase() {
	// Cylinder Base
	glColor3f(0.6, 0.3, 0.1); // Brown base
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.0);
	drawCylinderWithTexture(1.0, 1.0, 2.0, 50, 10);
	glPopMatrix();

}


void drawTowerStem() {
	glColor3f(0.0, 0.5, 0.0); // Green 
	glPushMatrix();
	glTranslatef(0.0, 0.0, 2.0); // Above the base
	drawCylinder(0.7, 0.4, 6.0, 50, 10);
	glPopMatrix();
}


void drawLotusTop() {
	// Lotus base sphere
	glColor3f(0.6, 0.1, 0.7); 
	glPushMatrix();
	glTranslatef(0.0, 0.0, 8.0); 
	drawSphere(1.0, 50, 50);
	glPopMatrix();

	//  top box 2
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 9.0);
	glScalef(2, 1, 1);
	glutSolidCube(.4);
	glPopMatrix();


	//  top box 2
	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 9.2);
	glutSolidCube(.5);
	glPopMatrix();

	// Golden spire on top
	glColor3f(1.0, 0.8, 0.0); // Golden color
	glPushMatrix();
	glTranslatef(0.0, 0.0, 9.5); // Above the lotus
	drawCylinder(0.1, 0.0, 1.5, 50, 10);
	glPopMatrix();


}


void drawTower() {
	glPushMatrix(); 
	glRotatef(-90, 1.0, 0, 0.0);
	drawTowerBase();
	drawTowerStem();
	drawLotusTop();
	glPopMatrix();
}





/////////////////////////////////////////////////////////////////////////////////////////////////



void backObjLoop() {
	float minX = roadLeft - 2.0f;  
	float maxX = roadRight + 2.0f; 
	float spacing = 15.0f;         


	for (float z = roadStart; z > roadEnd; z -= spacing) {

		// Left Side - Tower
		glPushMatrix();
		glTranslatef(minX, 0, z+2);  
		glScaled(0.3f, 0.3f, 0.3f);  
		drawTower();                 
		glPopMatrix();


		// Left Side - Building1
		glPushMatrix();
		glTranslatef(minX, 0, z);  
		glRotatef(90, 0, 1, 0.0);
		glScaled(0.5f, 0.5f, 0.5f);  
		drawBuilding1();             
		glPopMatrix();

		// Left Side - Building2
		glPushMatrix();
		glTranslatef(minX - 4, 0, z+5);
		glRotatef(90, 0, 1, 0.0);
		glScaled(0.5f, 0.5f, 0.5f);
		drawBuilding2();
		glPopMatrix();


		// Left Side - Building2
		glPushMatrix();
		glTranslatef(minX - 8, 0, z + 20);
		glRotatef(90, 0, 1, 0.0);
		glScaled(0.5f, 0.5f, 0.5f);
		drawSmallBuilding();
		glPopMatrix();


		// Left Side - Tree 01
		glPushMatrix();
		glTranslatef(minX - 3, 0, z + 3);
		glRotatef(-90, 1, 0, 0.0);
		glScaled(0.5f, 0.5f, 0.5f);
		drawTree1();
		glPopMatrix();

		// Left Side - Tree 02
		glPushMatrix();
		glTranslatef(minX , 1, z + 7);
		glScaled(0.5f, 0.5f, 0.5f);
		drawTree2();
		glPopMatrix();
		
		//////////////////////////////////////////////////////////

		// Right Side - Tower
		glPushMatrix();
		glTranslatef(maxX, 0, z + 2);  
		glScaled(0.3f, 0.3f, 0.3f); 
		drawTower();                 
		glPopMatrix();

		

		// Right Side - Building1
		glPushMatrix();
		glTranslatef(maxX, 0, z);  
		glScaled(0.5f, 0.5f, 0.5f);  
		drawBuilding1();            
		glPopMatrix();

		// Right Side - Building2
		glPushMatrix();
		glTranslatef(maxX + 4, 0, z+5);
		glScaled(0.5f, 0.5f, 0.5f);
		drawBuilding2();
		glPopMatrix();

		// Right Side - Tree 01
		glPushMatrix();
		glTranslatef(maxX + 3, 0, z + 2);
		glRotatef(-90, 1, 0, 0.0);
		glScaled(0.5f, 0.5f, 0.5f);
		drawTree1();
		glPopMatrix();

		// Right Side - Tree 02
		glPushMatrix();
		glTranslatef(maxX , 1, z + 7);
		glScaled(0.5f, 0.5f, 0.5f);
		drawTree2();
		glPopMatrix();


	}
}




float coinThickness = 0.05f;
float coinRadius = 0.13f;

void drawCoin() {
	GLUquadric* quad = gluNewQuadric();

	glPushMatrix();
	glTranslatef(0.0f, 0.1f, 0.0f);  

	
	gluQuadricTexture(quad, GL_TRUE);
	gluCylinder(quad, coinRadius, coinRadius, coinThickness, 50, 50);

	// Draw the top face
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, coinThickness);
	gluQuadricTexture(quad, GL_TRUE);
	gluDisk(quad, 0, coinRadius, 50, 50);
	glPopMatrix();

	// Draw the bottom face
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	gluQuadricTexture(quad, GL_TRUE);
	gluDisk(quad, 0, coinRadius, 50, 50);
	glPopMatrix();

	glPopMatrix();

	gluDeleteQuadric(quad);
}



void drawCoins() {
	glEnable(GL_LIGHTING);      // Enable lighting
	glEnable(GL_LIGHT0);
	for (int i = 0; i < coins.size(); i++) {
		if (coins[i].colided) {
			continue;
		}
		glPushMatrix();

		glTranslatef(coins[i].x, 0.3, coins[i].z);
		glRotatef(coinAngle, 0, 1, 0);

		if (coins[i].special) {   // Special coins
			GLfloat mat_ambient[] = { 0.8f, 0.2f, 0.2f, 1.0f };
			GLfloat mat_diffuse[] = { 0.8f, 0.2f, 0.2f, 1.0f };
			GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			GLfloat mat_shininess[] = { 50.0f };

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

			glColor3f(0.8, 0.2, 0.2);
			drawCoin();
		}
		else {     // Normal coins

			GLfloat torusAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
			GLfloat torusDiffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
			GLfloat torusSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			GLfloat torusShininess[] = { 50.0f };

			glMaterialfv(GL_FRONT, GL_AMBIENT, torusAmbient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, torusDiffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, torusSpecular);
			glMaterialfv(GL_FRONT, GL_SHININESS, torusShininess);

			glColor3f(1, 1, 0);
			drawCoin();
		}

		coinAngle += coinAngle < 360 ? 0.1 : -360;

		glPopMatrix();

	}
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
}





void drawObstacles() {
	float blockSize = 0.5;
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (int i = 0; i < obstacles.size(); i++) {
		float x1 = obstacles[i].x1, x2 = obstacles[i].x2, z = obstacles[i].z; x1 -= 0.19;
		glBegin(GL_QUADS);

		//back
		glTexCoord2f(0, 0); glVertex3f(x1, 0, z);
		glTexCoord2f(1, 0); glVertex3f(x2, 0, z);
		glTexCoord2f(1, 1); glVertex3f(x2, blockSize, z);
		glTexCoord2f(0, 1); glVertex3f(x1, blockSize, z);
		float wallDepth = 0.3;
		//front
		glTexCoord2f(0, 0); glVertex3f(x1, 0, z - wallDepth);
		glTexCoord2f(1, 0); glVertex3f(x2, 0, z - wallDepth);
		glTexCoord2f(1, 1); glVertex3f(x2, blockSize, z - wallDepth);
		glTexCoord2f(0, 1); glVertex3f(x1, blockSize, z - wallDepth);


		//left
		glTexCoord2f(1, 1); glVertex3f(x1, 0, z);
		glTexCoord2f(1, 0); glVertex3f(x1, blockSize, z);
		glTexCoord2f(0, 0); glVertex3f(x1, blockSize, z - wallDepth);
		glTexCoord2f(0, 1); glVertex3f(x1, 0, z - wallDepth);

		//right
		glTexCoord2f(0, 0); glVertex3f(x2, 0, z);
		glTexCoord2f(0, 1); glVertex3f(x2, blockSize, z);
		glTexCoord2f(1, 1); glVertex3f(x2, blockSize, z - wallDepth);
		glTexCoord2f(1, 0); glVertex3f(x2, 0, z - wallDepth);


		//top
		glTexCoord2f(1, 1); glVertex3f(x2, blockSize, z - wallDepth);
		glTexCoord2f(1, 0); glVertex3f(x2, blockSize, z);
		glTexCoord2f(0, 0); glVertex3f(x1, blockSize, z);
		glTexCoord2f(0, 1); glVertex3f(x1, blockSize, z - wallDepth);

		glEnd();
	}

}

void drawEnv() {

	// Road
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glBegin(GL_POLYGON);
	glTexCoord2d(0.0f, 10.0f); glVertex3f(roadRight, 0, roadEnd);
	glTexCoord2d(1.0f, 10.0f); glVertex3f(roadLeft, 0, roadEnd);
	glTexCoord2d(1.0f, 0.0f); glVertex3f(roadLeft, 0, roadStart);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(roadRight, 0, roadStart);
	glEnd();

	// Ground right
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glBegin(GL_POLYGON);
	glTexCoord2d(0.0f, 50.0f); glVertex3f(2, 0, roadEnd);
	glTexCoord2d(1.0f, 50.0f); glVertex3f(30, 0, roadEnd);
	glTexCoord2d(1.0f, 0.0f); glVertex3f(2, 0, roadStart);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(30, 0, roadStart);
	glEnd();

	// Ground left
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glBegin(GL_POLYGON);
	glTexCoord2d(0.0f, 50.0f); glVertex3f(-2.5, 0, roadEnd);
	glTexCoord2d(1.0f, 50.0f); glVertex3f(-30, 0, roadEnd);
	glTexCoord2d(1.0f, 0.0f); glVertex3f(-2.5, 0, roadStart);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(-30, 0, roadStart);
	glEnd();
	
	// Right side
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glBegin(GL_POLYGON);
	glTexCoord2d(0.0f, 1.0f); glVertex3f(20, 0, roadStart);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(20, 5, roadStart);
	glTexCoord2d(30.0f, 0.0f); glVertex3f(20, 5, roadEnd);
	glTexCoord2d(30.0f, 1.0f); glVertex3f(20, 0, roadEnd);
	glEnd();

	// Left side
	glBegin(GL_POLYGON);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexCoord2d(30.0f, 1.0f); glVertex3f(-20, 0, roadEnd);
	glTexCoord2d(30.0f, 0.0f); glVertex3f(-20, 5, roadEnd);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(-20, 5, roadStart);
	glTexCoord2d(0.0f, 1.0f); glVertex3f(-20, 0, roadStart);
	glEnd();
	

	// Sky
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glBegin(GL_POLYGON);
	glTexCoord2d(0.0f, 20.0f); glVertex3f(20, 5, roadEnd);
	glTexCoord2d(1.0f, 20.0f); glVertex3f(-20, 5, roadEnd);
	glTexCoord2d(1.0f, 0.0f); glVertex3f(-20, 5, roadStart);
	glTexCoord2d(0.0f, 0.0f); glVertex3f(20, 5, roadStart);
	glEnd();


	// End
	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glBegin(GL_POLYGON);
	glTexCoord2d(1.0f, 1.0f);
	glVertex3f(20, 0, roadEnd);
	glTexCoord2d(1.0f, 0.0f);
	glVertex3f(20, 5, roadEnd);
	glTexCoord2d(0.0f, 0.0f);
	glVertex3f(-20, 5, roadEnd);
	glTexCoord2d(0.0f, 1.0f);
	glVertex3f(-20, 0, roadEnd);
	glEnd();


}

void overlay(float rectWidth, float rectHeight, float xPos, float yPos, string text, float alpha) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, screenWidth, 0, screenHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0f, 0.0f, 0.0f, alpha);

	glBegin(GL_QUADS);
	glVertex2f(xPos, yPos);
	glVertex2f(xPos + rectWidth, yPos);
	glVertex2f(xPos + rectWidth, yPos + rectHeight);
	glVertex2f(xPos, yPos + rectHeight);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	float textXPos = xPos + rectWidth / 2 - rectWidth / 7, textYPos = yPos + rectHeight / 2 - 10;
	glRasterPos2f(textXPos, textYPos);
	for (char c : text) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void roadEndReached() {
	if (carZ < roadEnd + 1) {
		win = true;
	}
}



/////////////////////////////////////////////////////

void setLighting() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE); // Crucial for scaled objects
	glShadeModel(GL_SMOOTH);

	// Global ambient (keep it subtle)
	GLfloat globalAmbient[] = { 0.15f, 0.15f, 0.15f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	
	GLfloat light0Diffuse[] = { 0.9f, 0.9f, 0.85f, 1.0f }; 
	GLfloat light0Specular[] = { 1.0f, 1.0f, 0.9f, 1.0f };
	GLfloat light0Position[] = { -4.0f, 8.0f, -4.0f, 0.0f }; 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);

	// Set lighting intensity and color - light 1
	GLfloat qaAmbientLight1[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat qaDiffuseLight1[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat qaSpecularLight1[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, qaAmbientLight1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, qaDiffuseLight1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, qaSpecularLight1);

	// Material properties for buildings
	GLfloat buildingAmbient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat buildingDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	GLfloat buildingSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat shininess = 32.0f;

	glMaterialfv(GL_FRONT, GL_AMBIENT, buildingAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, buildingDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, buildingSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}



void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(cameraX, cameraY, cameraZ + 2, carX, 0, carZ, 0, 1, 0);

	setLighting();

	drawEnv();
	drawObstacles();

	glDisable(GL_TEXTURE_2D);

	backObjLoop();
	drawFence();

	drawCoins();

	collectedCoins += coinsCollision();
	overlay(250, 60, 1186, 704, "Score : " + to_string(collectedCoins), 0.7);

	if (obstaclesCollision()) {
		overlay(330, 100, 336, 564, "Game Over!", 0.8);
		gameover = true;
	}

	else if (win) { 
		overlay(300, 100, 336, 564, "You Win!", 0.8);
	}

	glColor3f(1, 1, 1);
	glPushMatrix();

	glTranslatef(carX, -0.088, carZ);
	(new Car)->draw(0.0f, 0.0f, 0.6f);
	glPopMatrix();

	glutSwapBuffers();
}



// Camera
void defaultCamera() { cameraX = carX - 0.45, cameraY = 1.3, cameraZ = carZ + 1.5; }



// Keyboard handling
void keyboardUp(unsigned char key, int, int) { // reset the buffer of a key if it is released
	buffer[tolower(key)] = false;
}

void keyboard(unsigned char key, int, int) {// set the buffer of a key if it is pressed
	if (key == ESC) {
		exit(0);
	}
	unsigned char k = tolower(key);
	buffer[k] = true;
	/*
	if (key == '!')
		glDisable(GL_LIGHT0);

	if (key == '1')
		glEnable(GL_LIGHT0);

	if (key == '@')
		glDisable(GL_LIGHT1);

	if (key == '2')
		glEnable(GL_LIGHT1);
	*/
}

void movement() { // if any key is pressed, an action occurs
	float spd;
	if (buffer['w']) { // move forward
		spd = carSpeed;
		if (carZ - spd >= roadEnd) {
			carZ -= spd;
			cameraZ -= spd;
		}
	}


	if (buffer['s']) { // move backward
		spd = carSpeed / 2;
		if (carZ + spd <= roadStart - 5) {
			carZ += spd;
			cameraZ += spd;
		}
	}

	if (buffer['d']) { // move right
		spd = carSpeed / 6;
		if (carX + spd <= roadRight + 0.11) {
			carX += spd;
			cameraX += spd;
		}
	}

	if (buffer['a']) { // move left
		spd = carSpeed / 6;
		if (carX - spd >= roadLeft + 0.7) {
			carX -= spd;
			cameraX -= spd;
		}

	}

	if (buffer['+']) {
		if (cameraZ - 0.2 > roadEnd) {
			cameraZ -= 0.2;
		}
	}

	else if (buffer['-']) {
		if (cameraZ + 0.2 < roadStart) {
			cameraZ += 0.2;
		}
	}

}

void specialKeys(int key, int, int) {

	if (key == GLUT_KEY_HOME) {
		defaultCamera();
	}
}




// Mouse handling
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouselastX = x;
		mouselastY = y;
	}
}

void motion(int x, int y) {
	int dx = x - mouselastX;
	int dy = y - mouselastY;

	float tx = cameraX - dx * 0.02;
	float ty = cameraY + dy * 0.02;

	if (tx <= roadRight - 0.1 && tx >= roadLeft + 0.1) {
		cameraX = tx;
	}
	if (ty <= 3 && ty >= 0.1) {
		cameraY = ty;
	}


	mouselastX = x;
	mouselastY = y;
}



// Basic functions
void init() {

	defaultCamera();

	srand(time(0));
	setCoins(100);
	setObstacles(40);

	glClearColor(0, 0, 0, 1.0f);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	

	loadTexture("road.bmp", 1);
	loadTexture("Sky_Texture-1057.jpg", 3);
	loadTexture("wall.jpg", 4);
	loadTexture("Ground2.jpg", 5);  //Ground.jpeg
	loadTexture("back2.jpg", 2);
	loadTexture("1153451.jpg", 6);
	//loadTexture("end.jpg", 7);

	glutFullScreen();
	ShowWindow(GetConsoleWindow(), 0);
}





void reshape(int width, int height) {

	if (height == 0) height = 1;
	aspect = float(width) / height;

	screenWidth = width;
	screenHeight = height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspect, 0.1f, 200);
	glMatrixMode(GL_MODELVIEW);

}


void timer(int) {

	movement();
	roadEndReached();
	glEnable(GL_TEXTURE_2D);
	glutPostRedisplay();

	if (gameover) {
		delay(4);
		exit(0);
	}

	if (win) {		
		delay(4);
		exit(0);
	}

	glutTimerFunc(5, timer, 0);
}


int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("3D Car Game");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(0, timer, 0);

	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutSpecialFunc(specialKeys);

	glutMainLoop();

	system("pause");
	return 0;
}
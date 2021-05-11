/**********************************/
/* Lighting					      
   (C) Bedrich Benes 2021         
   Diffuse and specular per fragment.
   bbenes@purdue.edu               */
/**********************************/

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <string>
#include <vector>			//Standard template library class
#include <GL/glew.h>
#include <GL/glut.h>
#include <json.hpp>

#include "SceneParser.h"

using json = nlohmann::json;
//OpenMP
#include <omp.h>

//glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/half_float.hpp>
#include "shaders.h"    
#include "shapes.h"    
#include "lights.h"    

#pragma warning(disable : 4996)
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "glut32.lib")

using namespace std;

bool needRedisplay=false;
ShapesC* sphere;

//hack: Smooth Demo mode constant allows to show smooth movement of particles


vector<glm::vec3> arrowPos;
vector<glm::vec3> particlesPos;
vector<float> particleCharges;


glm::vec3 eye = glm::vec3(0.f, 0.f, 10.f);
glm::vec3 eyeDir = glm::vec3(0.f, 0.f, -10.f);
glm::vec3 eyeVel = glm::vec3(0.f, 0.f, 0.f);



//shader program ID
GLuint shaderProgram;
GLfloat ftime=0.f;
glm::mat4 view=glm::mat4(1.0);
glm::mat4 proj=glm::perspective(80.0f,//fovy
				  		        1.0f,//aspect
						        0.01f,1000.f); //near, far
class ShaderParamsC
{
public:
	GLint modelParameter;		//modeling matrix
	GLint modelViewNParameter;  //modeliview for normals
	GLint viewParameter;		//viewing matrix
	GLint projParameter;		//projection matrix
	//material
	GLint kaParameter;			//ambient material
	GLint kdParameter;			//diffuse material
	GLint ksParameter;			//specular material
	GLint shParameter;			//shinenness material
} params;

//Todo: Add openMP parrelization
//Todo: make Powerpoint for presentation
LightC light;


//the main window size
GLint wWindow=800;
GLint hWindow=800;

float sh=1;

/*********************************
Some OpenGL-related functions
**********************************/

//called when a window is reshaped
void Reshape(int w, int h)
{
  glViewport(0,0,w, h);       
  glEnable(GL_DEPTH_TEST);
//remember the settings for the camera
  wWindow=w;
  hWindow=h;
}

void Arm(glm::mat4 m)
{
//let's use instancing
	m=glm::translate(m,glm::vec3(0,0.5,0.0));
	m=glm::scale(m,glm::vec3(1.0f,1.0f,1.0f));
	sphere->SetModel(m);
	//now the normals
	glm::mat3 modelViewN=glm::mat3(view*m);
	modelViewN= glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	sphere->Render();

	m=glm::translate(m,glm::vec3(0.0,0.5,0.0));
	m=glm::rotate(m,-20.0f*ftime,glm::vec3(0.0,0.0,1.0));
	m=glm::translate(m,glm::vec3(0.0,1.5,0.0));
	sphere->SetModel(glm::scale(m,glm::vec3(0.5f,1.0f,0.5f)));

	modelViewN=glm::mat3(view*m);
	modelViewN= glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	sphere->Render();
}
//Todo: Make arrow object correct shape
//Todo: Add arrow coloring option for mag
void arrow(glm::mat4 m, float scale, glm::vec3 dir)
{
	//let's use instancing
	m = glm::translate(m, glm::vec3(0, 0.5, 0.0));
	m = glm::scale(m, glm::vec3(0.2f, 0.2f, 0.2f));
	sphere->SetModel(m);
	//now the normals
	glm::mat3 modelViewN = glm::mat3(view * m);
	modelViewN = glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	sphere->SetKd(glm::vec3(0.5, 0.5, 0.5));
	sphere->Render();

	

	
	//m = glm::scale(m, glm::vec3(5.0f, 5.0f, 5.0f));
	//m = glm::translate(m, glm::vec3(0.0, 1.5, 0.0));
	m = glm::rotate(m, (float)(atan2(dir[1], dir[0]) * 180 / M_PI) + 90.0f, glm::vec3(0, 0, 1));
	//m = glm::rotate(m, 90.0f, glm::cross(dir, glm::vec3(0, 1, 0)));
	//m = glm::rotate(m, -20.0f * ftime, glm::vec3(0.0, 0.0, 1.0));
	//m = glm::translate(m, glm::vec3(dir[0]*5, dir[1]*5, dir[2]*5));
	sphere->SetModel(glm::scale(m, glm::vec3(1.0f, 2.5f, 1.0f)));
	//sphere->SetModel(glm::scale(m, dir));

	modelViewN = glm::mat3(view * m);
	modelViewN = glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	sphere->SetKd(glm::vec3(0.5, 0.5, 0.5));
	sphere->Render();


}

void particle(glm::mat4 m, float scale)
{
//let's use instancing
	m=glm::translate(m,glm::vec3(0,0.5,0.0));
	m=glm::scale(m,glm::vec3(scale, scale, scale));
	sphere->SetModel(m);
	//now the normals
	glm::mat3 modelViewN=glm::mat3(view*m);
	modelViewN= glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	sphere->SetKd(glm::vec3(0.5, 0.5, 0.5));
	sphere->Render();
	
	
}

void particle(glm::mat4 m, float scale, float charge)
{
	//let's use instancing
	m = glm::translate(m, glm::vec3(0, 0.5, 0.0));
	m = glm::scale(m, glm::vec3(scale, scale, scale));
	sphere->SetModel(m);
	//now the normals
	glm::mat3 modelViewN = glm::mat3(view * m);
	modelViewN = glm::transpose(glm::inverse(modelViewN));
	sphere->SetModelViewN(modelViewN);
	if (charge > 0) {
		sphere->SetKd(glm::vec3(1, 0, 0));
	}
	else if (charge < 0) {
		sphere->SetKd(glm::vec3(0, 0, 1));
	}
	else {
		sphere->SetKd(glm::vec3(1, 1, 1));
	}
	
	sphere->Render();
	


}

bool getParticleIntersection(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 particlePos, float particleRadius, glm::vec3& outIntersectionVector) {
	glm::vec3 rayDirNorm = glm::normalize(rayDir);
	float t;
	double xDif = rayOrigin[0] - particlePos[0];
	double yDif = rayOrigin[1] - particlePos[1];
	double zDif = rayOrigin[2] - particlePos[2];

	double b = 2 * (rayDirNorm.x * xDif + rayDirNorm.y * yDif + rayDirNorm.z * zDif);
	double c = xDif * xDif + yDif * yDif + zDif * zDif - particleRadius * particleRadius;

	double discriminant = b * b - 4 * c;

	//TODO: Deal with case of being inside the sphere or in front of it
	if (discriminant > 0) {
		t = (-1 * b - sqrt(discriminant)) / 2;
		outIntersectionVector = rayDirNorm * t + rayOrigin;
		return true;
	}
	else if (discriminant == 0) {
		t = -1 * b / 2;
		outIntersectionVector = rayDirNorm * t + rayOrigin;
		return true;
	}
	else {
		return false;
	}



}


//the main rendering function
void RenderObjects()
{
	const int range=3;
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glColor3f(0,0,0);
	glPointSize(2);
	glLineWidth(1);
	//set the projection and view once for the scene
	glUniformMatrix4fv(params.projParameter,1,GL_FALSE,glm::value_ptr(proj));
	//view=glm::lookAt(glm::vec3(25*sin(ftime/40.f),5.f,15*cos(ftime/40.f)),//eye
	//			     glm::vec3(0,0,0),  //destination
	//			     glm::vec3(0,1,0)); //up
	view=glm::lookAt(eye,//eye
				     eye + eyeDir,  //destination
				     glm::vec3(0,1,0)); //up
	eye = eye + eyeVel;
	glUniformMatrix4fv(params.viewParameter,1,GL_FALSE,glm::value_ptr(view));
//set the light
	static glm::vec4 pos;
	pos.x=20*sin(ftime/12);pos.y=-10;pos.z=20*cos(ftime/12);pos.w=1;
	
	light.SetPos(pos);
	light.SetShaders();
	for (int i=-range;i<range;i++)
	{
		for (int j=-range;j<range;j++)
		{
			glm::mat4 m=glm::translate(glm::mat4(1.0),glm::vec3(4*i,0,4*j));
			//Arm(m);
		}
	}


	//Todo: Remove arrow too close to the viewer
	//Render the positional Arrows
	int size = arrowPos.size();
	vector<glm::vec3> vecFields(size, glm::vec3(0.0, 0.0, 0.0));
#pragma omp parallel for private(n)
	for (int i = 0; i < arrowPos.size(); i++){
		//glm::mat4 m=glm::translate(glm::mat4(1.0),glm::vec3(arrowPos[i][0], arrowPos[i][1], arrowPos[i][2]));
		glm::vec3 fieldVec = glm::vec3(0.0, 0.0, 0.0);
		for (int n = 0; n < particlesPos.size(); n++) {
			//Electric Field Equation:
			glm::vec3 dir = arrowPos[i] - (particlesPos[n] + glm::vec3(4 * sin(ftime / 3), 4 * cos(ftime / 3), 0));
			float scale = 32.0;//Normally this would be 1/(4 pi Eo) in physics. Adjust to get the correct scale for viewing
			fieldVec += (scale * particleCharges[n] / pow(glm::length(dir), 2)) * glm::normalize(dir);

		}
		vecFields[i] = glm::normalize(fieldVec);
		
	}

	for (int i = 0; i < arrowPos.size(); i++) {
		glm::mat4 m = glm::translate(glm::mat4(1.0), glm::vec3(arrowPos[i][0], arrowPos[i][1], arrowPos[i][2]));
		arrow(m, 0.3, vecFields[i]);
	}
	//Render the particles

	for (int i =0; i < particlesPos.size(); i++){
		glm::mat4 m=glm::translate(glm::mat4(1.0),glm::vec3(particlesPos[i][0] + 4 * sin(ftime / 3), particlesPos[i][1] + 4 * cos(ftime / 3), particlesPos[i][2]));
		particle(m, 0.6, particleCharges[i]);
	}
}
	
void Idle(void)
{
  glClearColor(0.1,0.1,0.1,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  ftime+=0.05;
  glUseProgram(shaderProgram);
  RenderObjects();
  glutSwapBuffers();  
}

void Display(void)
{

}

//keyboard callback
void Kbd(unsigned char a, int x, int y)
{
	switch(a)
	{
 	  case 27 : exit(0);break;
	  case 'r': 
	  case 'R': {sphere->SetKd(glm::vec3(1,0,0));break;}
	  case 'g': 
	  case 'G': {sphere->SetKd(glm::vec3(0,1,0));break;}
	  case 'b': 
	  case 'B': {sphere->SetKd(glm::vec3(0,0,1));break;}
	  case '+': {sphere->SetSh(sh+=1);break;}
	  case '-': {sphere->SetSh(sh-=1);if (sh<1) sh=1;break;}
	}
	cout << "shineness="<<sh<<endl;
	glutPostRedisplay();
}


//special keyboard callback
void SpecKbdPress(int a, int x, int y)
{
	//Todo: upgrade movement through space.
   	switch(a)
	{
		case GLUT_KEY_LEFT:
		{
			eyeVel.x = -0.2;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			eyeVel.x = 0.2;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			eyeVel.z = 0.2;
			break;
		}
		case GLUT_KEY_UP:
		{
			eyeVel.z = -0.2;
			break;
		}
		case GLUT_KEY_PAGE_UP:
		{
			eyeVel.y = 0.2;
			break;
		}
		case GLUT_KEY_PAGE_DOWN:
		{
			eyeVel.y = -0.2;
			break;
		}
	}
	glutPostRedisplay();
}

//called when a special key is released
void SpecKbdRelease(int a, int x, int y)
{
	switch(a)
	{
		case GLUT_KEY_LEFT:
		{
			eyeVel.x = 0.0;
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			eyeVel.x = 0.0;
			break;
		}
		case GLUT_KEY_DOWN:
		{
			eyeVel.z = 0.0;
			break;
		}
		case GLUT_KEY_UP:
		{
			eyeVel.z = 0.0;
			break;
		}
		case GLUT_KEY_PAGE_UP:
		{
			eyeVel.y = 0.0;
			break;
		}
		case GLUT_KEY_PAGE_DOWN:
		{
			eyeVel.y = 0.0;
			break;
		}
	}
	glutPostRedisplay();
}


void Mouse(int button,int state,int x,int y)
{
	x = 400-x;
	glm::vec3 rayOrgin = eye;
	glm::vec3 rayDir = glm::vec3(x - 800 / 2, y - 800 / 2, eye[2] + ((double)800) / (2 * sin(90 / 2 * M_PI / 180)));;
	glm::vec3 outRay;
	//Todo: Add ability to move particles, Research how CAD packages do it
	for (int i = 0; i < particlesPos.size(); i++) {
		if (getParticleIntersection(rayOrgin, rayDir, particlesPos[i], 1.0, outRay)) {
			printf("shoot it!");
		}
	}
	
	cout << "Location is " << "[" << x << "'" << y << "]" << endl;
}


void InitializeProgram(GLuint *program)
{
	std::vector<GLuint> shaderList;

//load and compile shaders 	
	shaderList.push_back(CreateShader(GL_VERTEX_SHADER,   LoadShader("shaders/phong.vert")));
	shaderList.push_back(CreateShader(GL_FRAGMENT_SHADER, LoadShader("shaders/phong.frag")));

//create the shader program and attach the shaders to it
	*program = CreateProgram(shaderList);

//delete shaders (they are on the GPU now)
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	params.modelParameter=glGetUniformLocation(*program,"model");
	params.modelViewNParameter=glGetUniformLocation(*program,"modelViewN");
	params.viewParameter =glGetUniformLocation(*program,"view");
	params.projParameter =glGetUniformLocation(*program,"proj");
	//now the material properties
	params.kaParameter=glGetUniformLocation(*program,"mat.ka");
	params.kdParameter=glGetUniformLocation(*program,"mat.kd");
	params.ksParameter=glGetUniformLocation(*program,"mat.ks");
	params.shParameter=glGetUniformLocation(*program,"mat.sh");
	//now the light properties
	light.SetLaToShader(glGetUniformLocation(*program,"light.la"));
	light.SetLdToShader(glGetUniformLocation(*program,"light.ld"));
	light.SetLsToShader(glGetUniformLocation(*program,"light.ls"));
	light.SetLposToShader(glGetUniformLocation(*program,"light.lPos"));

	string fileName = "data/SampleData.json";
	//Read JSON file
    std::ifstream i(fileName, std::ifstream::binary);
    json j;
    i >> j;
	//HACK: Parser.h didn't work, added directly to main. 
	//SceneParser parser = SceneParser();
	//parser.parserJsons(fileName);
	cout << j["data3DSpace"] << "\n";
	printf("\n-----------\n");

	cout << j["data3DSpace"]["mapPositions"]["vertex1"] << "\n";
	
	

	//Parses Charges
	for (int i = 0; i < j["data3DSpace"]["counts"]["particuleCount"]; i++){
		float x = j["data3DSpace"]["particles"][i]["position"][0];
		float y = j["data3DSpace"]["particles"][i]["position"][1];
		float z = j["data3DSpace"]["particles"][i]["position"][2];
		

		float charge = j["data3DSpace"]["particles"][i]["relative_charge"];
		particlesPos.push_back(glm::vec3(x, y, z));
		particleCharges.push_back(charge);

		
	}

	//Parses ArrowVectors
	//Todo: Make Vectors a category instead of individual data.
	for (int i = 0; i < j["data3DSpace"]["counts"]["vertCount"]; i++){
		glm::vec3 newPos;
		float x = j["data3DSpace"]["mapPositions"]["vertex"][i]["position"][0];
		float y = j["data3DSpace"]["mapPositions"]["vertex"][i]["position"][1];
		float z = j["data3DSpace"]["mapPositions"]["vertex"][i]["position"][2];
		arrowPos.push_back(glm::vec3(x, y, z));
	}
	for (int i = 0; i < j["data3DSpace"]["counts"]["vertBoxCount"]; i++) {
		int lowerX = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["xOffset"][0];
		int upperX = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["xOffset"][1];
		int stepX = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["xOffset"][2];
		int lowerY = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["yOffset"][0];
		int upperY = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["yOffset"][1];
		int stepY = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["yOffset"][2];
		int lowerZ = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["zOffset"][0];
		int upperZ = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["zOffset"][1];
		int stepZ = j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["zOffset"][2];
		
		for (int z = lowerZ; z <= upperZ; z += stepZ) {
			for (int y = lowerY; y <= upperY; y += stepY) {
				for (int x = lowerX; x <= upperX; x += stepX) {
					glm::vec3 newPos;
					float xPos = x + j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["centerPos"][0];
					float yPos = y + j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["centerPos"][1];
					float zPos = z + j["data3DSpace"]["mapPositions"]["vertexBoxes"][i]["centerPos"][2];
					arrowPos.push_back(glm::vec3(xPos, yPos, zPos));
				}
			}
		}
		
	}
	



    string s = j.dump();
    printf("Printing the Json file:\n");
	printf(s.c_str());
	printf("\n-----------\n");
	


	

}

void InitShapes(ShaderParamsC *params)
{
//create one unit sphere in the origin
	sphere=new SphereC(50,50,1);
	sphere->SetKa(glm::vec3(0.1,0.1,0.1));
	sphere->SetKs(glm::vec3(0,0,1));
	sphere->SetKd(glm::vec3(0.7,0.7,0.7));
	sphere->SetSh(200);
	sphere->SetModel(glm::mat4(1.0));
	sphere->SetModelMatrixParamToShader(params->modelParameter);
	sphere->SetModelViewNMatrixParamToShader(params->modelViewNParameter);
	sphere->SetKaToShader(params->kaParameter);
	sphere->SetKdToShader(params->kdParameter);
	sphere->SetKsToShader(params->ksParameter);
	sphere->SetShToShader(params->shParameter);
}

int main(int argc, char **argv)
{ 
  glutInitDisplayString("stencil>=2 rgb double depth samples");
  glutInit(&argc, argv);
  glutInitWindowSize(wWindow,hWindow);
  glutInitWindowPosition(500,100);
  glutCreateWindow("Model View Projection GLSL");
  GLenum err = glewInit();
  if (GLEW_OK != err){
   fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutMouseFunc(Mouse);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(Kbd); //+ and -
  glutSpecialUpFunc(SpecKbdRelease); //smooth motion
  glutSpecialFunc(SpecKbdPress);
  InitializeProgram(&shaderProgram);
  InitShapes(&params);
  glutMainLoop();
  return 0;        
}
	
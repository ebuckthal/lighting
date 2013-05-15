/*
   CPE 471 Assignment 1
   Lighting.cpp
 */

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLSL_helper.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

#include "Mesh.h"

#define NUM_MATERIALS 2

using namespace std;
using namespace glm;

GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uSP;
GLint h_uMode;
GLint h_uDiffuse;
GLint h_uSpecular;
GLint h_uAmbient;
GLint h_uShininess;
GLuint vbo_mesh;
GLuint ibo_mesh;
GLuint nbo_mesh;
GLuint vbo_light;
GLuint ibo_light;

int i_size, l_size;

int ShadeProg;

float g_height;
float g_width;

float g_angle = 0;
vec3 g_trans(0,0, -10);

mat4 RM = mat4(1.0f);
mat4 oRM = mat4(1.0f);

bool g_selTranslate, g_selRotate, g_selScale, g_mousedown;
float g_basex, g_basey;
float g_transx = 0.0;
float g_transy = 0.0;
float g_otransx = 0.0;
float g_otransy = 0.0;
float g_scale = 1.0;
float g_oscale = 1.0;
int g_mode = 1;
int g_material = 0;

float g_shininess[NUM_MATERIALS] = {200.0, 4.0};
float g_diffuse[NUM_MATERIALS][3] = {{0, 0.08, 0.5}, {23.0/255, 99.0/255, 166.0/255}};
float g_ambient[NUM_MATERIALS][3] = {{0.2, 0.2, 0.2}, {0.1, 0.1, 0.1}};
float g_specular[NUM_MATERIALS][3] = {{0.4, 0.4, 0.4}, {3.0/255, 88.0/255, 140.0/255}};


vec3 g_sp(0,5,2);

void SetProjectionMatrix()
{
  glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

void SetView()
{
  glm::mat4 Trans = glm::translate( glm::mat4(1.0f), g_trans);
  glm::mat4 RotateX = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));
  safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
}


void drawLight(void)
{
  glUseProgram(ShadeProg);

  SetProjectionMatrix();
  SetView();
  
  mat4 TM = translate(mat4(1.0f), g_sp);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(mat4(TM)));

  glUniform3f(h_uSP, g_sp.x, g_sp.y, g_sp.z);
  glUniform1i(h_uMode, 3); //light mode

  safe_glEnableVertexAttribArray(h_aPosition);
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo_light);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_light);
  glDrawElements(GL_TRIANGLES, l_size, GL_UNSIGNED_INT, 0);
  
  safe_glDisableVertexAttribArray(h_aPosition);

  glUseProgram(0);

}

void drawMesh(void)
{
  glUseProgram(ShadeProg);

  SetProjectionMatrix();
  SetView();

  mat4 TM = translate(mat4(1.0f), vec3(g_transx, g_transy, 0));
  mat4 SM = scale(mat4(1.0f), vec3(g_scale));
  mat4 CTM = TM * SM * RM;
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(mat4(CTM)));

  glUniform3f(h_uSP, g_sp.x, g_sp.y, g_sp.z);
  glUniform1i(h_uMode, g_mode);

  glUniform3f(h_uAmbient, g_ambient[g_material][0], g_ambient[g_material][1], g_ambient[g_material][2]);
  glUniform3f(h_uDiffuse, g_diffuse[g_material][0], g_diffuse[g_material][1], g_diffuse[g_material][2]);
  glUniform3f(h_uSpecular, g_specular[g_material][0],g_specular[g_material][1],g_specular[g_material][2]);
  glUniform1f(h_uShininess, g_shininess[g_material]);
  
  safe_glEnableVertexAttribArray(h_aPosition);
  safe_glEnableVertexAttribArray(h_aNormal);
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh);
  safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, nbo_mesh);
  safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh);
  glDrawElements(GL_TRIANGLES, i_size, GL_UNSIGNED_INT, 0);
  
  safe_glDisableVertexAttribArray(h_aPosition);
  safe_glDisableVertexAttribArray(h_aNormal);

  glUseProgram(0);
}

int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName)
{
  GLuint VS; //handles to shader object
  GLuint FS; //handles to frag shader object
  GLint vCompiled, fCompiled, linked; //status of shader

  VS = glCreateShader(GL_VERTEX_SHADER);
  FS = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(VS, 1, &vShaderName, NULL);
  glShaderSource(FS, 1, &fShaderName, NULL);

  //compile shader and print log
  glCompileShader(VS);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
  printShaderInfoLog(VS);

  //compile shader and print log
  glCompileShader(FS);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
  printShaderInfoLog(FS);

  if (!vCompiled || !fCompiled) {
    printf("Error compiling either shader\n\n%s\n\nor\n\n%s\n\n", vShaderName, fShaderName);
    return -1;
  }

  //create a program object and attach the compiled shader
  ShadeProg = glCreateProgram();
  glAttachShader(ShadeProg, VS);
  glAttachShader(ShadeProg, FS);

  glLinkProgram(ShadeProg);
  /* check shader status requires helper functions */
  printOpenGLError();
  glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
  //printProgramInfoLog(ShadeProg);

  glUseProgram(ShadeProg);

  /* get handles to attribute data */
  h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
  h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
  h_uSP= safe_glGetUniformLocation(ShadeProg,  "uSP");
  h_uMode= safe_glGetUniformLocation(ShadeProg,  "uMode");
  h_uAmbient = safe_glGetUniformLocation(ShadeProg,  "uAmbient");
  h_uDiffuse = safe_glGetUniformLocation(ShadeProg,  "uDiffuse");
  h_uSpecular = safe_glGetUniformLocation(ShadeProg,  "uSpecular");
  h_uShininess = safe_glGetUniformLocation(ShadeProg,  "uShininess");
  h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
  h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
  h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
  //printf("sucessfully installed shader %d\n", ShadeProg);
  return 1;
}

void Draw(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawLight();
  drawMesh();

  glutSwapBuffers();
}

int initMesh(Mesh m)
{

  int i = 0;
  float vbo[m.vertices.size()*3];
  for(map<int,vec3>::iterator it = m.vertices.begin();
      it != m.vertices.end();
      it++)
  {
    vbo[i++] = it->second.x;
    vbo[i++] = it->second.y;
    vbo[i++] = it->second.z;
  }

  i = 0;
  unsigned int ibo[m.triangles.size()*3];
  for(map<int, vector<int> >::iterator it=m.triangles.begin();
      it != m.triangles.end();
      it++)
  {
    ibo[i++] = it->second.at(0)-1;
    ibo[i++] = it->second.at(1)-1;
    ibo[i++] = it->second.at(2)-1;
  }
  i_size = m.triangles.size()*3;

  i = 0;
  float nbo[m.verNormals.size()*3];
  for(map<int, vec3>::iterator it = m.verNormals.begin();
      it != m.verNormals.end();
      it++)
  {
    nbo[i++] = it->second.x;
    nbo[i++] = it->second.y;
    nbo[i++] = it->second.z;
  }

  glGenBuffers(1, &vbo_mesh);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_mesh);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vbo), vbo, GL_STATIC_DRAW);

  glGenBuffers(1, &ibo_mesh);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_mesh);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo), ibo, GL_STATIC_DRAW);

  glGenBuffers(1, &nbo_mesh);
  glBindBuffer(GL_ARRAY_BUFFER, nbo_mesh);
  glBufferData(GL_ARRAY_BUFFER, sizeof(nbo), nbo, GL_STATIC_DRAW);

  return 0;
}

void initLight(void) {

  float vbo[] = {
    -0.1, -0.1, -0.1,
    -0.1, 0.1, -0.1,
    0.1, 0.1, -0.1,
    0.1, -0.1, -0.1,
    0.0, 0.1, -0.1,
    -0.1, -0.1, 0.1,
    -0.1, 0.1, 0.1,
    0.1, 0.1, 0.1,
    0.1, -0.1, 0.1,
    0.0, 0.1, 0.1,
    -0.1, -0.1, 0.1,
    -0.1, -0.1, -.1,
    -0.1, 0.1, -0.1,
    -0.1, 0.1, 0.1,
    0.1, -0.1, 0.1,
    0.1, -0.1, -.1,
    0.1, 0.1, -0.1,
    0.1, 0.1, 0.1,
    };

    unsigned int ibo[] = {
    0, 1, 2,
    2, 3, 0,
    1, 4, 2,
    5, 6, 7,
    7, 8, 5,
    6, 9, 7,
    10, 11, 12,
    12, 13, 10,
    14, 15, 16,
    16, 17, 14,
    };

    l_size = 30;
    glGenBuffers(1, &vbo_light);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_light);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vbo), vbo, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo_light);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_light);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ibo), ibo, GL_STATIC_DRAW);
   
};

void ReshapeGL(int width, int height)
{
  g_width = (float)width;
  g_height = (float)height;
  glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));
}

void Initialize()
{
  glClearColor(84.0/255, 191.0/255, 131.0/255, 1.0f);

  glClearDepth(1.0f);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key)
  {
  case 't':
    g_selTranslate = !g_selTranslate;
    g_selRotate = false;
    g_selScale = false;
    break;
  case 'r':
    g_selRotate = !g_selRotate;
    g_selTranslate = false;
    g_selScale = false;
    break;
  case 's':
    g_selScale = !g_selScale;
    g_selTranslate = false;
    g_selRotate = false;
    break;
  case 'a':
    break;
  case 'x':
    g_sp.x++;
    break;
  case 'z':
    g_sp.x--;
    break;
  case 'b':
    g_mode = 1;
    break;
  case 'v':
    g_mode = 2;
    break;
  case 'n':
    g_mode = 0;
    break;
  case 'y':
    g_material = (++g_material)%NUM_MATERIALS;
    break;
  case 'q':
    exit(0);
    break;
  }
  glutPostRedisplay();
}

vec3 getTrackballVector(float xi, float yi)
{
  float vx, vy, vz;

  if(sqrt((xi*xi)+(yi*yi)) <= 1)
  {
    vx = xi;
    vy = yi;
    vz = sqrt((1-(xi*xi)-(yi*yi)));
  }
  else
  {
    vx = xi / sqrt((xi*xi)+(yi*yi));
    vy = yi / sqrt((xi*xi)+(yi*yi));
    vz = 0;
  }

  return vec3(vx, vy, vz);
}

float distance(float x1, float x2, float y1, float y2)
{
  return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void motion(int x, int y)
{
  //if cube is selected, all array indices are 0, 1 for roof, return if neither is selected

  int width = glutGet(GLUT_WINDOW_WIDTH);
  int height = glutGet(GLUT_WINDOW_HEIGHT);

  y = (height - y);

  //current in image coord
  float xi = x/(width/2.0) - 1;
  float yi = y/(height/2.0) - 1;

  //base in image coord
  float xi_base = g_basex/(width/2.0) - 1;
  float yi_base = g_basey/(height/2.0) - 1;

  //center of cube in image coord
  float xi_center = g_transx/5.0;
  float yi_center = g_transy/5.0;

  //distance from mousedown position to center of cube in image coords
  float dist_base = distance(xi_base, xi_center, yi_base, yi_center);

  //distance from center of cube to current image coordinates
  float dist_cur = distance(xi, xi_center, yi, yi_center);

  if(g_selScale)
  {
    //new scale = old scale * (% distance mouse cursor from center / distance base from center)
    g_scale = g_oscale * (dist_cur/dist_base);
  }
  else if(g_selTranslate)
  {
    g_transx = g_otransx + 5*(xi-xi_base);
    g_transy = g_otransy + 5*(yi-yi_base);

  }
  else if(g_selRotate)
  {

    //getTrackballVector gets the vector made by x,y image coordinates and assumed z
    vec3 now = getTrackballVector(xi, yi);
    vec3 start = getTrackballVector(xi_base, yi_base);

    //calculate angle between two vectors
    float angle = acos(dot(start, now)) * (180.0/3.14159);

    //calculate normal angle to perform rotation on
    vec3 n = cross(start, now);

    //if angle > 0, create new rotate matrix to avoid disappearing cube bug
    RM = (angle > 0 ? rotate(mat4(1.0f), angle, n) : oRM);
    RM = RM * oRM;

  }
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  int height = glutGet(GLUT_WINDOW_HEIGHT);
  if(state == 0) //down
  {
    g_mousedown = true;

    //base = starting position in pixels
    g_basex = x;
    g_basey = (height - y);
  }
  else if(state == 1) //up
  {
    g_mousedown = false;

    //save this mousedown-worth of manipulation in a really ugly way
    g_otransx = g_transx;
    g_otransy = g_transy;
    g_oscale = g_scale;
    oRM = RM;
  }
}

void printUsage(void)
{
  cout << "HOW TO USE:-------------------------------------" << endl;
  cout << "r - toggles rotation of mesh" << endl;
  cout << "t - toggles translation of mesh" << endl;
  cout << "s - toggles scale of mesh" << endl;
  cout << "------------------------------------------------" << endl;
  cout << "z - moves light source left" << endl;
  cout << "x - moves light source right" << endl;
  cout << "------------------------------------------------" << endl;
  cout << "y - cycles through pre-set materials" << endl;
  cout << "v - sets shader to Phong shading (per pixel)" << endl;
  cout << "b - sets shader to Gouraud shading (per vertex)" << endl;
  cout << "n - sets shader to color model with normals" << endl;
  cout << "------------------------------------------------" << endl;
}


int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitWindowPosition(20, 20);
  glutInitWindowSize(600, 600);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Lighting");
  glutReshapeFunc(ReshapeGL);
  glutDisplayFunc(Draw);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  Initialize();

  getGLversion();

  if(!(InstallShader(textFileRead((char*)"vert.glsl"), textFileRead((char*)"frag.glsl"))))
  {
    perror("Error installing shader");
    exit(-1);
  }


  Mesh m = Mesh();
  m.init(argv[1]);
  m.center(vec3(0,0,0));
  m.scale(vec3(4,4,4));
  m.computeNormals();
  initMesh(m);
  initLight();

  printUsage();

  glutMainLoop();
  return 0;
}




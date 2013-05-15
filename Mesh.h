/*
   CPE 471 Assignment 1
   Mesh.h
 */

#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class Mesh {

private:

public:
   Mesh();
   ~Mesh();

   std::map<int, glm::vec3> vertices; //vert # -> x,y,z coord
   std::map<int, std::vector<int> > triangles; //tri # -> included verts
   std::map<int, glm::vec3> triNormals;
   std::map<int, glm::vec3> verNormals;

   int init(char* filename);
   int center(glm::vec3 c);
   int scale(glm::vec3 s);
   int computeNormals();
   glm::vec3 computeNormal(std::vector<int> tri);

};

#endif

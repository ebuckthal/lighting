/*
   CPE 471 Assignment 1
   Mesh.cpp
 */

#include "Mesh.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>
#include <set>
#include <algorithm>

using namespace std;
using namespace glm;

Mesh::Mesh() {};

Mesh::~Mesh() {};

int Mesh::scale(vec3 s)
{

  map<int, vec3>::iterator it=vertices.begin();
  vec3 min = it->second;
  vec3 max = it->second;

  for(; it != vertices.end(); it++)
  {
    if(min.x > it->second.x) {min.x = it->second.x;}
    if(min.y > it->second.y) {min.y = it->second.y;}
    if(min.z > it->second.z) {min.z = it->second.z;}

    if(max.x < it->second.x) {max.x = it->second.x;}
    if(max.y < it->second.y) {max.y = it->second.y;}
    if(max.z > it->second.z) {max.z = it->second.z;}

  }

  float extent = std::max(max.x - min.x, std::max(max.y-min.y, max.z-min.z));

  vec3 resize;
  resize.x = s.x / extent;
  resize.y = s.y / extent;
  resize.z = s.z / extent;

  for(map<int, vec3>::iterator it=vertices.begin();
      it != vertices.end();
      it++)
  {
    it->second.x = (it->second.x * resize.x);
    it->second.y = (it->second.y * resize.y);
    it->second.z = (it->second.z * resize.z);
  }

  return 0;
}

int Mesh::center(vec3 c)
{
  map<int, vec3>::iterator it=vertices.begin();
  vec3 min = it->second;
  vec3 max = it->second;

  for(; it != vertices.end(); it++)
  {
    if(min.x > it->second.x) {min.x = it->second.x;}
    if(min.y > it->second.y) {min.y = it->second.y;}

    if(max.x < it->second.x) {max.x = it->second.x;}
    if(max.y < it->second.y) {max.y = it->second.y;}

  }

  vec3 center;
  center.x = c.x - ((max.x + min.x) / 2);
  center.y = c.y - ((max.y + min.y) / 2);

  for(map<int, vec3>::iterator it=vertices.begin();
      it != vertices.end();
      it++)
  {
    it->second.x = (it->second.x + center.x);
    it->second.y = (it->second.y + center.y);

  }

  return 0;
};


int Mesh::init(char* filename)
{
  std::ifstream file;
  file.open(filename, std::ifstream::in);

  while(file)
  {
    string readline;
    getline(file, readline);

    stringstream Stream(readline);

    string label;
    Stream >> label;

    if(label.find("#") != string::npos)
    {
      continue;
    }

    if("Vertex" == label)
    {
      int index;
      vec3 v;

      Stream >> index;

      Stream >> v.x;
      Stream >> v.y;
      Stream >> v.z;

      vertices.insert(make_pair(index, v));
      verNormals.insert(make_pair(index, vec3(0,0,0)));
    }
    else if("Face" == label)
    {
      int index;
      int v1, v2, v3;
      std::vector<int> v;

      Stream >> index;

      Stream >> v1;
      Stream >> v2;
      Stream >> v3;

      v.push_back(v1);
      v.push_back(v2);
      v.push_back(v3);

      triangles.insert(make_pair(index, v));
    }
    else if("" == label)
    {
      continue;
    }
    else
    {
      cerr << "Error while parsing ASCII mesh: expected 'Vertex' or 'Face', found '" << label << "'" << endl;
    }
  }

  return 0;
}

vec3 Mesh::computeNormal(vector<int> tri)
{
  vec3 const a = vertices.at(tri.at(0));
  vec3 const b = vertices.at(tri.at(1));
  vec3 const c = vertices.at(tri.at(2));

  return normalize(glm::cross(b - a, c - a));
}

int Mesh::computeNormals()
{
  
  //compute the normals for every triangle
  for(map<int, vector<int> >::iterator it=triangles.begin();
      it != triangles.end();
      it++)
  {
    vec3 n = computeNormal(it->second);
    
    for(vector<int>::iterator vs=it->second.begin();
        vs != it->second.end();
        vs++)
    {
      verNormals.at(*vs) += n;
    }
  }

  /*for(map<int, vec3>::iterator it = verNormals.begin();
      it != verNormals.end();
      it++)
  {
    it->second = normalize(it->second);
  }*/

  return 0;
}

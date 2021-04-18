#pragma once
#include<stdio.h>
#include <json.hpp>
#include <string>

//Glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/half_float.hpp>

#include <iostream>

using namespace std;

class SceneParser {
public:
    vector<glm::vec3> arrowPos;
    vector<glm::vec3> particlePos;
    vector<float> particleCharge;
    void parserJsons(string fileName);




};

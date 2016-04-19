//
// Created by jakub on 4/16/16.
//

#ifndef IC_MATH_H
#define IC_MATH_H

#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace ifc {
    const float PI_OVER_180 = M_PI/180;

    float dot(const glm::vec3 &v1, const glm::vec3 &v2);

    float euclideanDistance(const glm::vec3& v1, const glm::vec3& v2);

    float angleToRadians(float angle);

    void solveTridiagonalSystem(std::vector<float>&belowDiagonal, std::vector<float>&mainDiagonal,
                                std::vector<float>&aboveDiagonal, std::vector<float>& d);
}

#endif //IC_MATH_H
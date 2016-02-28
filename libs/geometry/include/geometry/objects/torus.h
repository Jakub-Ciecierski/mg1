//
// Created by jakub on 2/26/16.
//

#ifndef MG1_TORUS_H
#define MG1_TORUS_H

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <geometry/edge.h>
#include <geometry/renderable.h>

class Torus : public Renderable{
private:
    //-----------------------------------------------------------//
    //  PRIVATE FIELDS
    //-----------------------------------------------------------//

    float innerRadius;
    float outerRadiusr;

    unsigned int sidesCount;
    unsigned int ringsCount;

    //-----------------------------------------------------------//
    //  PRIVATE METHODS
    //-----------------------------------------------------------//

    float getX(float innerRadius, float outerRadiusr);
    float getY(float innerRadius, float outerRadiusr);
    float getZ(float innerRadius, float outerRadiusr);

protected:
    //-----------------------------------------------------------//
    //  PROTECTED METHODS
    //-----------------------------------------------------------//

    virtual void initVertices() override;
    virtual void initEdges() override;

public:
    //-----------------------------------------------------------//
    //  CONSTRUCTORS
    //-----------------------------------------------------------//

    Torus(float innerRadius, float outerRadiusr);

    Torus(float innerRadius, float outerRadiusr,
          unsigned int sidesCount,
          unsigned int ringsCount);

    ~Torus();
};


#endif //MG1_TORUS_H

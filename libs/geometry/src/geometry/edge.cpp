//
// Created by jakub on 2/26/16.
//

#include <geometry/edge.h>

using namespace glm;

Edge::Edge(fvec4 *vertex1, fvec4 *vertex2) :
    vertex1(vertex1), vertex2(vertex2){

}

Edge::Edge(unsigned int i, unsigned int j) :
    i(i), j(j){

}

unsigned int Edge::getVertex1() const{
    return i;
}

unsigned int Edge::getVertex2() const{
    return j;
}

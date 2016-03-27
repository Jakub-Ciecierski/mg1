//
// Created by jakub on 2/28/16.
//

#include <GL/gl.h>
#include <gm/color/color_settings.h>
#include <gm/rendering/render_body.h>

using namespace glm;
using namespace std;

//-----------------------------------------------------------//
//  CONSTRUCTORS
//-----------------------------------------------------------//

RenderBody::RenderBody() : color(COLOR_OBJECT_DEFAULT){
    lineWidth = 1.0f;
    grabable = true;
}

RenderBody::~RenderBody() {

}

//-----------------------------------------------------------//
//  PRIVATE METHODS
//-----------------------------------------------------------//

void RenderBody::setSurfaceColor(const Color &color) {
    glColor3f(color.R, color.G, color.B);
}

const vector<vec4>& RenderBody::getVertices() {
    return this->vertices;
}

const vector<Edge>& RenderBody::getEdges() {
    return this->edges;
}

void RenderBody::transformPosition(const glm::mat4 &VP){
    vec4 transformedOrigins(getPosition(), 1);

    transformedOrigins = VP * transformedOrigins;
    transformedOrigins.x /= transformedOrigins.w;
    transformedOrigins.y /= transformedOrigins.w;
    transformedOrigins.z /= transformedOrigins.w;

    projectedPosition.x = transformedOrigins.x;
    projectedPosition.y = transformedOrigins.y;
    projectedPosition.z = transformedOrigins.z;
}

void RenderBody::transform(const mat4 &VP){
    mat4 MVP = VP * getModelMatrix();
    transformPosition(VP);

    for(unsigned int i = 0; i < transformedVertices.size(); i++){
        transformedVertices[i] = MVP * vertices[i];

        transformedVertices[i].x /= transformedVertices[i].w;
        transformedVertices[i].y /= transformedVertices[i].w;
        transformedVertices[i].z /= transformedVertices[i].w;
    }
}

void RenderBody::drawLines(const std::vector<glm::vec4>& vertices,
                           bool costumColor){
    const vector<Edge>& edges = getEdges();

    glLineWidth((GLfloat)lineWidth);
    glBegin(GL_LINES);
    for(unsigned int i = 0; i < edges.size(); i++){
        if(costumColor && edges[i].color != NULL)
            setSurfaceColor(*(edges[i].color));

        unsigned int index1 = edges[i].getVertex1();
        unsigned int index2 = edges[i].getVertex2();
        vec4 v1 = vertices[index1];
        vec4 v2 = vertices[index2];

        // CLIPPING
        // Don't draw if either is not visiable
        if(v1.w < 0 || v2.w < 0) continue;

        // v2 is behind
        if(v2.w < 0){
            float n = (v1.w - 1.0) / (v1.w - v2.w);
            float xc = (n * v1.x) + ((1-n) * v2.x);
            float yc = (n * v1.y) + ((1-n) * v2.y);

            v2 = vec4(xc,yc,0,0);
        }
        // v1 is behind
        if(v1.w < 0){
            float n = (v2.w - 1.0f) / (v2.w - v1.w);
            float xc = (n * v2.x) + ((1-n) * v1.x);
            float yc = (n * v2.y) + ((1-n) * v1.y);

            v1 = vec4(xc,yc,0,0);
        }
        glVertex2f(v1.x,
                   v1.y);
        glVertex2f(v2.x,
                   v2.y);
    }
    glEnd();
}

//-----------------------------------------------------------//
//  PUBLIC METHODS
//-----------------------------------------------------------//

bool RenderBody::isGrabable() {
    return grabable;
}

void RenderBody::setColor(Color color) {
    this->color = color;
}

const vec3& RenderBody::getProjectedPosition(){
    return this->projectedPosition;
}

const Color* RenderBody::getColor() {
    return &color;
}


void RenderBody::render(const mat4 &VP) {
    transform(VP);

    setSurfaceColor(this->color);
    drawLines(transformedVertices, true);
}

void RenderBody::render(const glm::mat4 &VP, const Color &color) {
    transform(VP);

    setSurfaceColor(color);
    drawLines(transformedVertices, false);
}


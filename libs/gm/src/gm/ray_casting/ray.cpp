//
// Created by jakub on 3/6/16.
//

#include <gm/ray_casting/ray.h>
#include <GL/gl.h>
#include <gm/ray_casting/ray_constants.h>
#include <iostream>
#include <gm/utils.h>

using namespace glm;

Ray::Ray(Renderer* renderer) : renderer(renderer) {
    resetAdaptiveRayCasting();
}

void Ray::rayCasting(Renderer& renderer) {
    Scene* scene = renderer.getScene();

    unsigned int width = renderer.getWindowWidth();
    unsigned int height = renderer.getWindowHeight();

    ellipsoid->updateDMVPMatrix(scene->getMVP());

    glBegin(GL_POINTS);
    for(unsigned int x = 0; x < width; x++){
        float xGL = renderer.xPixelToGLCoord(x);
        for(unsigned int y = 0; y < height; y++){
            float yGL = renderer.yPixelToGLCoord(y);

            //float z = ellipsoid->intersect(xGL, yGL);
            float z = ellipsoid->intersect(xGL, yGL,
                                           scene->getActiveCamera()->getPosition());

            if(z == RAY_NO_SOLUTION){
                const Color& c = scene->getColor();
                glColor3f(c.R, c.G, c.B);
            }
            else {
                Color surfaceColor = computeLightIntensity(vec3(xGL,yGL, z));
                glColor3f(surfaceColor.R, surfaceColor.G, surfaceColor.B);
            }

            glVertex2f(xGL,
                       yGL);
        }
    }
    glEnd();
}

bool Ray::adaptiveRayCasting(Renderer &renderer, int exponent) {
    bool maxTileCountReached = false;

    Scene* scene = renderer.getScene();
    ellipsoid->updateDMVPMatrix(scene->getMVP());

    unsigned int width = renderer.getWindowWidth();
    unsigned int height = renderer.getWindowHeight();

    computeTiledWindow(width, height,
                       tileWidthCount, tileHeightCount);

    if(tileWidthCount == width && tileHeightCount == height)
        maxTileCountReached = true;

    tileWidthCount *= exponent;
    tileHeightCount *= exponent;

    if(tileWidthCount > width) {
        tileWidthCount = width;
    }
    if(tileHeightCount > height) {
        tileHeightCount = height;
    }

    return maxTileCountReached;
}

void Ray::computeTiledWindow(int width, int height,
                             int tileWidthCount, int tileHeightCount) {
    // add the rest to the last tile
    int restWidth = width % tileWidthCount;
    int restHeight = height % tileHeightCount;

/*
    std::cout << "---------------------------" << std::endl;
    std::cout << "tileWidthCount: " << tileWidthCount << std::endl;
    std::cout << "tileHeightCount: " << tileHeightCount << std::endl;
*/
    for(int i = 0; i < tileWidthCount; i++){
        int currentRestWidth = 0;
        int tileWidth = width / tileWidthCount;
        if(i == 0) {
            currentRestWidth = restWidth;
            //tileWidth += restWidth;
        }

        for(int j = 0; j < tileHeightCount; j++){
            int currentRestHeight = 0;
            int tileHeight = height / tileHeightCount;
            if(j == 0) {
                currentRestHeight = restHeight;
                //tileHeight += restHeight;
            }
            computeTile(i, j, tileWidth, tileHeight,
                        currentRestWidth, currentRestHeight);
        }
    }
}


void Ray::computeTile(int xtileID, int ytileID,
                      int tileWidth, int tileHeight,
                      int restWidth, int restHeight){
    int xStart = xtileID * tileWidth;
    int yStart = ytileID * tileHeight;

    tileWidth += restWidth;
    tileHeight += restHeight;

    float xStartGL = renderer->xPixelToGLCoord(xStart);
    float yStartGL = renderer->yPixelToGLCoord(yStart);

    // Compute intersectiong of first pixel in tile

    float z = ellipsoid->intersect(xStartGL, yStartGL,
                                   renderer->getScene()->
                                           getActiveCamera()->getPosition());
    glBegin(GL_POINTS);
    if(z == RAY_NO_SOLUTION){
        const Color& c = renderer->getScene()->getColor();
        glColor3f(c.R, c.G, c.B);
    }
    else {
        Color c = computeLightIntensity(vec3(xStartGL, yStartGL, z));
        glColor3f(c.R, c.G, c.B);
    }
    for(int x = xStart; x < xStart + tileWidth; x++){
        for(int y = yStart; y < yStart + tileHeight; y++){
            glVertex2f(renderer->xPixelToGLCoord(x),
                       renderer->yPixelToGLCoord(y));
        }
    }
    glEnd();
}

Color Ray::computeLightIntensity(const vec3& p) {
    const vec3& eyePosition =
            renderer->getScene()->getActiveCamera()->getPosition();
    vec3 N = normalize(ellipsoid->derivative(p));
    vec3 V = normalize(eyePosition - p);

/*
    std::cout << std::endl << "-----------" << std::endl;
    std::cout << "Intersection" << std::endl;
    printvec3(p);
    std::cout << "Eye" << std::endl;
    printvec3(eyePosition);
    std::cout << "Normal:" << std::endl;
    printvec3(N);
    std::cout << "Towards View" << std::endl;
    printvec3(V);
*/

    float dot = (V.x * N.x) + (V.y * N.y) + (V.z * N.z);
    if (dot < 0) dot = 0;
    float lightIntensity = pow(dot, intesityExponent);

/*
    std::cout << "dot: " << dot << std::endl;
    std::cout << "intesityExponent: " << intesityExponent << std::endl;
    std::cout << "lightIntensity: " << lightIntensity << std::endl;
*/
    const Color& c = ellipsoid->getColor();

    Color surfaceColor = Color(c.R * lightIntensity,
                               c.G * lightIntensity,
                               c.B * lightIntensity,
                               c.Alpha);

    return surfaceColor;
}

void Ray::resetAdaptiveRayCasting() {
    tileWidthCount = 16;
    tileHeightCount = 16;
}

bool Ray::adaptiveRayCasting_Parallel(Renderer &renderer, int exponent) {
    Scene* scene = renderer.getScene();
    // shouldn't be here
    ellipsoid->updateDMVPMatrix(scene->getMVP());

    unsigned int width = renderer.getWindowWidth();
    unsigned int height = renderer.getWindowHeight();

    std::vector<TileWindow> tileWindows;
    int id;
    // create Tiles
    bool doneCreating = false;
    do{
        if(tileWidthCount == width && tileHeightCount == height)
            doneCreating = true;

        tileWidthCount *= exponent;
        tileHeightCount *= exponent;
        if(tileWidthCount > width) tileWidthCount = width;
        if(tileHeightCount > height) tileHeightCount = height;

        TileWindow tileWindow;
        tileWindow.id = id;
        tileWindow.widthWindow = width;
        tileWindow.heightWindow = height;
        tileWindow.tileWidthCount = tileWidthCount;
        tileWindow.tileHeightCount = tileHeightCount;
    }while(!doneCreating);
}

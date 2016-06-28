//
// Created by jakub on 6/25/16.
//

#ifndef IC_SURFACE_FILLING_H
#define IC_SURFACE_FILLING_H

#include <infinity_cad/rendering/render_objects/surfaces/surface.h>
#include <infinity_cad/rendering/scene/scene.h>

enum BorderCurveParam{
    U, V, U0, U1, V0, V1
};

enum RenderMode{
    DEBUG, NORMAL
};

enum FillingStatus{
    COMPUTING, UNKNOWN_ERROR, COMPLETE, NO_HOLE
};

struct CommonPoints{
    ifc::Point* point12;
    ifc::Point* point13;
    ifc::Point* point23;
};

struct FillingData{
    Surface* surface;

    FillingData* left;
    FillingData* right;

    // Commons Points shared between neighbour border.
    ifc::Point* commonPoint1;
    ifc::Point* commonPoint2;

    // Border points make up border curve.
    std::vector<ifc::Point*> borderPoints;

    // Is the curve drawn by u or v parameter
    BorderCurveParam varBorderCurveParam;
    BorderCurveParam constBorderCurveParam;

    glm::vec3 borderCurveTanget;

    glm::vec3 a0Tanget;
    glm::vec3 b0Tanget;
    glm::vec3 a3Tanget;
    glm::vec3 b3Tanget;

    glm::vec3 a0Point;
    glm::vec3 b0Point;
    glm::vec3 a3Point;
    glm::vec3 b3Point;

    // P0
    glm::vec3 P0_BorderCurveMidPoint;
    // P1 = P0 + borderCurveTanget
    glm::vec3 P1_Tanget;
    // P2
    glm::vec3 P2;
    // P3
    glm::vec3 P3_Center;
};

struct DebugColors{
    Color CommonPointColor;
    Color BorderControlPointsColor;

    Color BorderCurveMidPointColor;
    Color BorderCurveMidTangetColor;

    Color a0TangentColor;
    Color a3TangentColor;
    Color b3TangentColor;

    Color P2Color;
    Color P3Color;

    Color CenterBezierColor;
};

/*
 * Assumes that Surfaces are C0 single patches
 */
class SurfaceFilling {
private:
    FillingData fillingData[3];

    Scene* scene;

    DebugColors debugColors;
    RenderMode renderMode;
    CommonPoints commonPoints;
    FillingStatus fillingStatus;

    // ----------------

    /*
     * Finds Common control points among the surfaces
     */
    void findCommonPoints();
    ifc::Point* findCommonPoint(Surface* surface1, Surface* surface2);
    void updateStatusCommonPoints(CommonPoints &commonPoints);

    /*
     * Finds the border curve of each surface
     */
    void findBorderCurves();
    void findBorderCurve(FillingData &fillingData);

    /*
     * Computes P0
     */
    void computeBorderCurveMidPoints();
    void computerBorderCurveMidPoint(FillingData& fillingData);

    /*
     * Computes P1
     */
    void computeBorderCurveTangents();
    void computeBorderCurveTangent(FillingData& fillingData);

    /*
     * Computes P2, P3
     */
    void computeCenterPoints();
    glm::vec3 computeAuxiliaryPointQ(FillingData &fillingData);

    /*
     * Computes a0, b0, a3, b3, g0,g1,g2
     */
    void computeGFieldVectors();
    void computeGFieldVector(FillingData &fillingData);

    glm::vec2 getUV(FillingData& fillingData);

    void renderDebug();

    std::string borderCurveParamToString(BorderCurveParam param);
public:

    SurfaceFilling(Surface* surface1, Surface* surface2, Surface* surface3,
                   Scene* scene);

    ~SurfaceFilling();

    FillingStatus getStatus();

    void start();
};


#endif //IC_SURFACE_FILLING_H
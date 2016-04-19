#include <infinity_cad/rendering/render_objects/curves/bspline_interp.h>
#include <infinity_cad/math/math.h>
#include <infinity_cad/geometry/polynomials/bspline_basis.h>
#include <infinity_cad/settings/settings.h>
#include <ifc_gpu/splines/bspline_gpu.h>

using namespace std;
using namespace glm;

//-----------------------//
//  CONSTRUCTORS
//-----------------------//

BSplineInterp::BSplineInterp(SceneID sceneID, std::string name) :
    Spline(sceneID, name){
    curvePoints = (vec4*)malloc(sizeof(vec4) * MAX_PIXEL_COUNT);
}

BSplineInterp::~BSplineInterp(){
    delete curvePoints;
}

//-----------------------//
//  PRIVATE
//-----------------------//

void BSplineInterp::computeChordParameters(){
    int n = points.size();
    parameters.clear();
    parameters.resize(n);
    float lengthOfPolygon = 0;
    for(int i = 1; i < n ; i++){
        float dist = ifc::euclideanDistance(points[i]->getPosition(),
                                            points[i-1]->getPosition());
        lengthOfPolygon += dist;
    }
    parameters[0] = 0;
    for(int i = 1; i < n -1; i++){
        float sumOfDistances = 0;
        for(int k = 1; k < i; k++){
            float dist = ifc::euclideanDistance(points[k]->getPosition(),
                                                points[k-1]->getPosition());
            sumOfDistances += dist;
        }
        parameters[i] = sumOfDistances / lengthOfPolygon;
    }
    parameters[n-1] = 1;
}

void BSplineInterp::computeKnotVector(){
    int n = points.size();
    int knotVectorCount = n + DEGREE + 1;

    knotVector.clear();
    knotVector.resize(knotVectorCount);

    for(int i = 0; i <= DEGREE; i++){
        knotVector[i] = 0;
    }
    int j;
    for(j = 1; j < n-DEGREE; j++){
        float sum = 0;
        for(int i = j; i < j + DEGREE; i++){
            sum += parameters[i];
        }
        knotVector[j+ DEGREE] = sum / DEGREE;
    }
    for(int i = j+DEGREE; i < knotVectorCount; i++){
        knotVector[i] = 1;
    }
}

void BSplineInterp::computeControlPoints(){
    int n = points.size();
    vector<float> belowDiagonal(n);
    vector<float> mainDiagonal(n);
    vector<float> aboveDiagonal(n);

    belowDiagonal[0] = 0;
    for(int i = 1; i < n; i++){
        belowDiagonal[i] = bsplineRecurive(parameters[i], DEGREE, i-1,
                                           knotVector);
    }
    for(int i = 0; i < n ;i++){
        mainDiagonal[i] = bsplineRecurive(parameters[i], DEGREE, i, knotVector);
    }
    for(int i = 0; i < n-1; i++){
        aboveDiagonal[i] = bsplineRecurive(parameters[i], DEGREE, i+1,
                                           knotVector);
    }
    aboveDiagonal[n-1] = 0;

    controlPoints.clear();
    controlPoints.resize(n);
    for(int s = 0; s < DIMENSION; s++){
        vector<float> d(n);
        vector<float> aboveDiagonalTMP(aboveDiagonal);

        for(int i = 0; i < n; i++){
            const vec3& pos = points[i]->getPosition();
            d[i] = pos[s];
        }

        ifc::solveTridiagonalSystem(belowDiagonal, mainDiagonal,
                                    aboveDiagonalTMP, d);

        for(int i = 0; i < n; i++){
            vec3& v = controlPoints[i];
            v[s] = d[i];
        }
    }
}
void BSplineInterp::drawSplineCPU(const glm::mat4 &VP, const Color& color,
                            int degree, float t, float t_max, float dt){
    setSurfaceColor(color);
    glPointSize(1.0f);
    glBegin(GL_POINTS);

    while(t < t_max){
        vec4 point = computeBSpline(controlPoints, knotVector, t, degree);
        t += dt;

        point = VP * point;
        point.x /= point.w;
        point.y /= point.w;

        if(point.w < 0) continue; // clip
        glVertex2f(point.x, point.y);
    }

    glEnd();
}
void BSplineInterp::drawSplineGPU(const glm::mat4 &VP, const Color& color,
                            int pixelCount, float t, float dt, int degree){
    int pointSize = controlPoints.size();

    if(pixelCount > MAX_PIXEL_COUNT){
        delete curvePoints;
        MAX_PIXEL_COUNT = pixelCount;
        curvePoints = (vec4*) malloc(MAX_PIXEL_COUNT * sizeof(vec4));
    }

    ifc_gpu::computeBSpline(controlPoints.data(), pointSize,
                            knotVector.data(), knotVector.size(),
                            curvePoints, pixelCount, t, dt, degree, &VP);

    setSurfaceColor(color);
    glPointSize(1.0f);
    glBegin(GL_POINTS);

    for(int i = 0; i < pixelCount; i++){
        if(curvePoints[i].w < 0) continue; // clip
        glVertex2f(curvePoints[i].x, curvePoints[i].y);
    }

    glEnd();
}

//-----------------------//
//  PROTECTED
//-----------------------//

void BSplineInterp::draw(const glm::mat4 &VP, const Color& color) {
    buildCurve();

    if(controlPoints.size() < 4) return;

    float t_max = 1.0f;
    float t = 0.0f;
    float dt = 0.001;
    int pixelCount = (t_max - t) / dt;

    if(ifc::RUN_CUDA){
        drawSplineGPU(VP, color, pixelCount, t, dt, DEGREE);
    }else{
        drawSplineCPU(VP, color, DEGREE, t, t_max, dt);
    }

}

void BSplineInterp::buildCurve() {
    computeChordParameters();
    computeKnotVector();
    computeControlPoints();
}


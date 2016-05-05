//
// Created by jakub on 5/4/16.
//

#include <infinity_cad/rendering/render_objects/surfaces/surface_c0_cylind.h>
#include <infinity_cad/rendering/scene/object_factory.h>
#include <infinity_cad/settings/settings.h>
#include <infinity_cad/geometry/polynomials/bernstein_basis.h>
#include <infinity_cad/math/math.h>
#include <algorithm>
#include <ifc_gpu/surfaces/bezier_surface_gpu.h>

using namespace glm;
using namespace std;

//-----------------------//
//  CONSTRUCTORS
//-----------------------//

SurfaceC0Cylind::SurfaceC0Cylind(SceneID id, std::string name,
                             int n, int m, float radius, float height) :
        Surface(id, name, n, m), radius(radius), height(height){
    this->build();
}

SurfaceC0Cylind::~SurfaceC0Cylind(){
    for(int i = 0; i < n;i++){
        for(int j = 0; j < m; j++){
            delete patches[i][j];
        }
    }
}

//-----------------------//
//  PRIVATE
//-----------------------//

Matrix<ifc::Point*> SurfaceC0Cylind::getMatrixFormOfAllPatches(){
    int numberOfPointsInRow = CUBIC_COUNT*n - (n-1) - 1;
    int numberOfPointsInColumns = CUBIC_COUNT*m - (m-1);

    vector<vector<ifc::Point*>> points(numberOfPointsInColumns);

    for(int i = 0; i < n; i++){
        int currentRow = 0;
        for(int j = 0; j < m; j++){
            BicubicBezierPatch* patch = patches[i][j];
            const Matrix<ifc::Point*>& controlPoints = patch->getPoints();

            int k = 0;
            if(j > 0) k = 1;
            for(; k < CUBIC_COUNT; k++){
                vector<ifc::Point*> row = controlPoints[k];
                for(int l = 0; l < CUBIC_COUNT; l++){
                    ifc::Point* point = row[l];

                    if(!(std::find(points[currentRow].begin(),
                                 points[currentRow].end(), point )
                       != points[currentRow].end())){
                        points[currentRow].push_back(point);
                    }
                }
                currentRow++;
            }
        }
    }
    Matrix<ifc::Point*> pointsMatrix(points,
                                     numberOfPointsInColumns,
                                     numberOfPointsInRow);
    return pointsMatrix;
}

void SurfaceC0Cylind::shapeTheCylidner(const Matrix<ifc::Point*>&rowWisePoints){
    for(unsigned int i = 0; i < rowWisePoints.rowCount(); i++){
        const vector<ifc::Point*>& row = rowWisePoints[i];
        unsigned int size = row.size();
        float da = (2*M_PI) / (float)size;
        float a = 0;
        float x,z;
        for(unsigned int j = 0; j < size; j++){
            x = sin(a) * radius;
            z = cos(a) * radius;

            row[j]->moveTo(x, row[j]->getPosition().y, z);

            a += da;
        }
    }
}

BicubicBezierPatch* SurfaceC0Cylind::createPatch(int n, int m,
                                               float patchHeight,
                                               const vec3& origin){
    float dY = patchHeight / (float)(CUBIC_COUNT - 1);

    ObjectFactory& objectFactory = ObjectFactory::getInstance();
    Matrix<ifc::Point*> points = initC0Points(n,m);

    float x,y,z;
    x = z = 0;

    for(int i = 0; i < CUBIC_COUNT; i++){ // column
        y = m*patchHeight;
        if(m > 0) y+=dY;

        for(int j = 0; j < CUBIC_COUNT; j++ ){ // row
            if(points[j][i] != NULL) continue;

            points[j][i] = objectFactory.createPoint(createPointName(n,m,j,i));
            allPoints.push_back(points[j][i]);
            components.push_back(points[j][i]);

            points[j][i]->moveTo(origin);
            points[j][i]->move(x, y, z);
            y += dY;
        }
    }
    BicubicBezierPatch* patch = new BicubicBezierPatch(points);

    return patch;
}

Matrix<ifc::Point*> SurfaceC0Cylind::initC0Points(int n, int m){
    Matrix<ifc::Point*> points(CUBIC_COUNT, CUBIC_COUNT, NULL);

    if (n > 0){
        BicubicBezierPatch* patch = patches[n-1][m];
        const Matrix<ifc::Point*>& prevPoints = patch->getPoints();
        points.setColumn(0, prevPoints.getColumn(3));

        if(n == this->n - 1){
            BicubicBezierPatch* patch = patches[0][m];
            const Matrix<ifc::Point*>& prevPoints = patch->getPoints();
            points.setColumn(3, prevPoints.getColumn(0));
        }
    }
    if (m > 0){
        BicubicBezierPatch* patch = patches[n][m-1];
        const Matrix<ifc::Point*>& prevPoints = patch->getPoints();
        points[0] = prevPoints[3];
    }

    return points;
}

//-----------------------//
//  PROTECTED
//-----------------------//

void SurfaceC0Cylind::build() {
    vec3 startPos(0,0,0);

    float patchHeight = height/(float)m;

    for(int i = 0; i < n;i++){
        for(int j = 0; j < m; j++){
            patches[i][j] = createPatch(i, j, patchHeight, startPos);
        }
    }

    Matrix<ifc::Point*> rowWisePoints = getMatrixFormOfAllPatches();
    shapeTheCylidner(rowWisePoints);
}
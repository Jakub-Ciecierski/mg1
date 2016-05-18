//
// Created by jakub on 5/15/16.
//

#ifndef IC_SURFACE_C2_CYLIND_H
#define IC_SURFACE_C2_CYLIND_H


#include "infinity_cad/rendering/render_objects/surfaces/surface.h"

class SurfaceC2Cylind : public Surface {
private:
    Matrix<ifc::Point*>* deboorPoints;

    float radius;
    float height;

protected:
    virtual void build() override;

    virtual void drawPolygon(const glm::mat4& VP, int segments = 1) override;
public:

    SurfaceC2Cylind(SceneID id, std::string name,
                  int n, int m,
                  float radius, float height);

    ~SurfaceC2Cylind();

    virtual void update() override;

};


#endif //IC_SURFACE_C2_CYLIND_H
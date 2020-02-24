#pragma once

#include "rsurface_types.h"

namespace rsurfaces
{
using namespace geometrycentral;

class TPEKernel
{
public:
    TPEKernel(MeshPtr m, GeomPtr g, double alpha, double beta);
    double tpe_pair(GCFace f1, GCFace f2);
    Vector3 tpe_gradient_pair(GCFace f1, GCFace f2, GCVertex wrt);

    void numericalTest();

    MeshPtr mesh;
    GeomPtr geom;
    double alpha, beta;

    private:
    double tpe_Kf(GCFace f1, GCFace f2);
    Vector3 tpe_gradient_Kf(GCFace f1, GCFace f2, GCVertex wrt);
    Vector3 tpe_gradient_Kf_num(GCFace f1, GCFace f2, GCVertex wrt, double eps);

};

} // namespace rsurfaces

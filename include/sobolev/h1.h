#pragma once

#include "rsurface_types.h"
#include "matrix_utils.h"

namespace rsurfaces
{
    namespace H1
    {
        void getTriplets(std::vector<Triplet> &triplets, MeshPtr &mesh, GeomPtr &geom);
        void ProjectGradient(Eigen::MatrixXd &gradient, Eigen::MatrixXd &dest, MeshPtr &mesh, GeomPtr &geom);
    }
} // namespace rsurfaces
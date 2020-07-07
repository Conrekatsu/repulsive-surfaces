#include "energy/all_pairs_tpe.h"
#include "helpers.h"

#include <unordered_set>

namespace rsurfaces
{

    AllPairsTPEnergy::AllPairsTPEnergy(TPEKernel *kernel_)
    {
        kernel = kernel_;
    }

    double AllPairsTPEnergy::Value()
    {
        double total = 0;
        for (GCFace f1 : kernel->mesh->faces())
        {
            for (GCFace f2 : kernel->mesh->faces())
            {
                if (f1 == f2)
                    continue;
                total += kernel->tpe_pair(f1, f2);
            }
        }
        return total;
    }

    void AllPairsTPEnergy::Differential(Eigen::MatrixXd &output)
    {
        surface::VertexData<size_t> indices = kernel->mesh->getVertexIndices();
        for (GCFace f1 : kernel->mesh->faces())
        {
            for (GCFace f2 : kernel->mesh->faces())
            {
                if (f1 == f2)
                    continue;
                // Find the set of vertices that are on the boundary of either
                // triangle, without duplicates
                std::vector<GCVertex> verts;
                GetVerticesWithoutDuplicates(f1, f2, verts);

                for (GCVertex &v : verts)
                {
                    Vector3 deriv1 = kernel->tpe_gradient_pair(f1, f2, v);
                    Vector3 sum = deriv1;
                    size_t r = indices[v];

                    output(r, 0) += sum.x;
                    output(r, 1) += sum.y;
                    output(r, 2) += sum.z;
                }
            }
        }
    }

    MeshPtr AllPairsTPEnergy::GetMesh()
    {
        return kernel->mesh;
    }

    GeomPtr AllPairsTPEnergy::GetGeom()
    {
        return kernel->geom;
    }

    Vector2 AllPairsTPEnergy::GetExponents()
    {
        return Vector2{kernel->alpha, kernel->beta};
    }

} // namespace rsurfaces
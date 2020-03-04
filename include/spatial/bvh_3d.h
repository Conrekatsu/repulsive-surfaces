#pragma once

#include "rsurface_types.h"

namespace rsurfaces
{

struct MassPoint
{
    double mass;
    Vector3 point;
    size_t elementID;
};

enum class BVHNodeType
{
    Empty,
    Leaf,
    Interior
};

class BVHNode3D
{
public:
    // Build a BVH of the given points
    BVHNode3D(std::vector<MassPoint> &points, int axis, BVHNode3D *root);
    ~BVHNode3D();

    // Basic spatial data
    double totalMass;
    Vector3 centerOfMass;
    Vector3 minCoords;
    Vector3 maxCoords;
    // Vector3 averageTangent;
    size_t elementID;
    // Every node knows the root of the tree
    BVHNode3D *bvhRoot;
    // Children
    std::vector<BVHNode3D *> children;
    BVHNodeType nodeType;
    int splitAxis;
    double splitPoint;
    double thresholdTheta;

    // Recursively recompute all centers of mass in this tree
    void recomputeCentersOfMass(MeshPtr &mesh, GeomPtr &geom);
    bool shouldUseCell(Vector3 vertPos);
    void printSummary();

private:
    double AxisSplittingPlane(std::vector<MassPoint> &points, int axis);
    void averageDataFromChildren();

    inline double nodeRatio(double d)
    {
        // Compute diagonal distance from corner to corner
        Vector3 diag = maxCoords - minCoords;
        double maxCoord = fmax(diag.x, fmax(diag.y, diag.z));
        return diag.norm() / d;
    }
};

BVHNode3D *CreateBVHFromMesh(MeshPtr &mesh, GeomPtr &geom);
} // namespace rsurfaces
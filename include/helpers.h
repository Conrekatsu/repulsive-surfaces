#pragma once

#include "rsurface_types.h"
#include <vector>
#include <unordered_set>
#include <chrono>

namespace rsurfaces
{

// Find the set of vertices that are on the boundary of either
// triangle, without duplicates
inline void GetVerticesWithoutDuplicates(GCFace f1, GCFace f2, std::vector<GCVertex> &verts)
{
    std::unordered_set<GCVertex> vertInds;
    for (GCVertex v : f1.adjacentVertices())
    {
        if (vertInds.count(v) == 0)
        {
            verts.push_back(v);
            vertInds.insert(v);
        }
    }
    for (GCVertex v : f2.adjacentVertices())
    {
        if (vertInds.count(v) == 0)
        {
            verts.push_back(v);
            vertInds.insert(v);
        }
    }
}

inline Vector3 GetRow(Eigen::MatrixXd &A, int i)
{
    return Vector3{A(i, 0), A(i, 1), A(i, 2)};
}

inline long currentTimeMilliseconds()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline Vector3 faceBarycenter(GeomPtr const &geom, GCFace f)
{
    Vector3 sum{0, 0, 0};
    int count = 0;
    for (GCVertex v : f.adjacentVertices())
    {
        sum += geom->inputVertexPositions[v];
        count++;
    }
    return sum / count;
}

inline Vector3 vectorMin(Vector3 v1, Vector3 v2)
{
    return Vector3{fmin(v1.x, v2.x), fmin(v1.y, v2.y), fmin(v1.z, v2.z)};
}

inline Vector3 vectorMax(Vector3 v1, Vector3 v2)
{
    return Vector3{fmax(v1.x, v2.x), fmax(v1.y, v2.y), fmax(v1.z, v2.z)};
}

} // namespace rsurfaces
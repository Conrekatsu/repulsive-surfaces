#pragma once

#include "rsurface_types.h"
#include "surface_flow.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "energy/barnes_hut_tpe_6d.h"
#include "scene_file.h"

#include "energy/squared_error.h"

namespace rsurfaces
{
    struct PriorityVertex
    {
        GCVertex vertex;
        double priority;
        Vector3 position;
    };

    class MainApp
    {
    public:
        static MainApp *instance;
        MainApp(MeshPtr mesh_, GeomPtr geom_, SurfaceFlow *flow_, polyscope::SurfaceMesh *psMesh_, std::string meshName_);

        void TestLML();
        void TestMVProduct();
        void BenchmarkBH();
        void TestBarnesHut();
        void PlotEnergyPerFace();
        void Scale2x();
        void TestNormalDeriv();

        void GetFalloffWindow(GCVertex v, double radius, std::vector<PriorityVertex> &verts);
        void HandlePicking();

        void TakeNaiveStep(double t);
        void TakeFractionalSobolevStep();
        void AddObstacle(std::string filename, double weight);
        void AddPotential(scene::PotentialType pType, double weight);

        MeshPtr mesh;
        GeomPtr geom;
        SurfaceFlow *flow;
        TPEKernel *kernel;
        polyscope::SurfaceMesh *psMesh;
        std::vector<polyscope::SurfaceMesh *> obstacles;
        std::string meshName;

        inline void reregisterMesh()
        {
            psMesh = polyscope::registerSurfaceMesh(meshName, geom->inputVertexPositions, mesh->getFaceVertexList());
        }
        
        void updateMeshPositions();

        BVHNode6D *vertBVH;
        bool normalizeView;
        double bh_theta;

    private:
        GCVertex pickedVertex;
        std::vector<PriorityVertex> dragVertices;

        double pickDepth;
        bool pickNearbyVertex(GCVertex &out);
        SquaredError* vertexPotential;
        bool ctrlMouseDown;
        Vector3 initialPickedPosition;
        bool hasPickedVertex;
    };
} // namespace rsurfaces

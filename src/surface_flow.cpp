#include "surface_flow.h"
#include "fractional_laplacian.h"
#include "helpers.h"

namespace rsurfaces
{

SurfaceFlow::SurfaceFlow(SurfaceEnergy *energy_)
{
    energy = energy_;
    mesh = energy->GetMesh();
    geom = energy->GetGeom();
}

void SurfaceFlow::StepNaive(double t)
{
    double energyBefore = energy->Value();
    Eigen::MatrixXd gradient;
    gradient.setZero(mesh->nVertices(), 3);

    energy->Differential(gradient);

    surface::VertexData<size_t> indices = mesh->getVertexIndices();

    for (GCVertex v : mesh->vertices())
    {
        Vector3 grad_v = GetRow(gradient, indices[v]);
        geom->inputVertexPositions[v] -= grad_v * t;
    }

    double energyAfter = energy->Value();

    std::cout << "Energy: " << energyBefore << " -> " << energyAfter << std::endl;
}

void SurfaceFlow::StepLineSearch()
{
    double energyBefore = energy->Value();
    Eigen::MatrixXd gradient;
    gradient.setZero(mesh->nVertices(), 3);

    energy->Differential(gradient);
    double initGuess = 1.0 / gradient.norm();

    surface::VertexData<size_t> indices = mesh->getVertexIndices();

    LineSearchStep(gradient, initGuess, 1);

    double energyAfter = energy->Value();

    std::cout << "Energy: " << energyBefore << " -> " << energyAfter << std::endl;
}


SurfaceEnergy *SurfaceFlow::BaseEnergy()
{
    return energy;
}

void SurfaceFlow::SaveCurrentPositions()
{
    surface::VertexData<size_t> indices = mesh->getVertexIndices();
    origPositions.setZero(mesh->nVertices(), 3);
    for (GCVertex v : mesh->vertices())
    {
        // Copy the vertex positions of each vertex into
        // the corresponding row
        Vector3 pos_v = geom->inputVertexPositions[v];
        size_t ind_v = indices[v];
        origPositions(ind_v, 0) = pos_v.x;
        origPositions(ind_v, 1) = pos_v.y;
        origPositions(ind_v, 2) = pos_v.z;
    }
}

void SurfaceFlow::RestorePositions()
{
    surface::VertexData<size_t> indices = mesh->getVertexIndices();
    for (GCVertex v : mesh->vertices())
    {
        // Copy the positions in each row of origPositions
        // back to each vertex
        size_t ind_v = indices[v];
        Vector3 pos_v = GetRow(origPositions, ind_v);
        geom->inputVertexPositions[v] = pos_v;
    }
}

void SurfaceFlow::SetGradientStep(Eigen::MatrixXd &gradient, double delta)
{
    surface::VertexData<size_t> indices = mesh->getVertexIndices();
    for (GCVertex v : mesh->vertices())
    {
        // Set the position of each vertex to be the sum of
        // origPositions + delta * gradient
        size_t ind_v = indices[v];
        Vector3 pos_v = GetRow(origPositions, ind_v);
        Vector3 grad_v = GetRow(gradient, ind_v);
        geom->inputVertexPositions[v] = pos_v - delta * grad_v;
    }
}

double SurfaceFlow::LineSearchStep(Eigen::MatrixXd &gradient, double initGuess, double gradDot)
{
    double delta = initGuess;
    SaveCurrentPositions();

    // Gather some initial data
    double initialEnergy = energy->Value();
    double gradNorm = gradient.norm();
    int numBacktracks = 0;
    double sigma = 0.01;
    double nextEnergy = initialEnergy;

    if (gradNorm < 1e-10)
    {
        std::cout << "Gradient is very close to zero" << std::endl;
        return 0;
    }

    while (delta > LS_STEP_THRESHOLD)
    {
        // Take the gradient step
        SetGradientStep(gradient, delta);
        nextEnergy = energy->Value();
        double decrease = initialEnergy - nextEnergy;
        double targetDecrease = sigma * delta * gradNorm * gradDot;

        if (decrease < targetDecrease)
        {
            delta /= 2;
            numBacktracks++;
        }
        // Otherwise, accept the current step.
        else
        {
            break;
        }
    }

    if (delta <= LS_STEP_THRESHOLD)
    {
        std::cout << "Failed to find a non-trivial step after " << numBacktracks << " backtracks" << std::endl;
        // Restore initial positions if step size goes to 0
        RestorePositions();
        return 0;
    }
    else
    {
        std::cout << "Took step of size " << delta << " after " << numBacktracks << " backtracks" << std::endl;
        return delta;
    }
}

} // namespace rsurfaces
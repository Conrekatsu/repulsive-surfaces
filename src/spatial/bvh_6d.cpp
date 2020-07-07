#include "spatial/bvh_6d.h"
#include "helpers.h"

namespace rsurfaces
{

inline int NextAxis(int axis)
{
    switch (axis)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 0;
    default:
        return 0;
    }
}

inline MassNormalPoint meshFaceToBody(const GCFace &f, GeomPtr &geom, FaceIndices &indices)
{
    Vector3 pos = faceBarycenter(geom, f);
    double mass = geom->faceArea(f);
    Vector3 n = geom->faceNormal(f);

    return MassNormalPoint{mass, n, pos, indices[f]};
}

inline double GetCoordFromBody(MassNormalPoint &mp, int axis)
{
    switch (axis)
    {
    case 0:
        return mp.point.x;
    case 1:
        return mp.point.y;
    case 2:
        return mp.point.z;
    case 3:
        return mp.normal.x;
    case 4:
        return mp.normal.y;
    case 5:
        return mp.normal.z;
    default:
        std::cout << "Invalid axis passed to GetCoordFromBody: " << axis << std::endl;
        exit(1);
        return 0;
    }
}

BVHNode6D *Create6DBVHFromMesh(MeshPtr &mesh, GeomPtr &geom)
{
    std::vector<MassNormalPoint> verts(mesh->nFaces());
    FaceIndices indices = mesh->getFaceIndices();

    // Loop over all the vertices
    for (const GCFace &f : mesh->faces())
    {
        MassNormalPoint curBody = meshFaceToBody(f, geom, indices);
        // Put vertex body into full list
        verts[curBody.elementID] = curBody;
    }

    BVHNode6D *tree = new BVHNode6D(verts, 0, 0);
    tree->assignIDsRecursively(0);
    return tree;
}

BVHNode6D::BVHNode6D(std::vector<MassNormalPoint> &points, int axis, BVHNode6D *root)
{
    // Split the points into sets somehow
    thresholdTheta = 0.25;
    splitAxis = axis;

    if (!root)
        bvhRoot = this;
    else
        bvhRoot = root;

    // If we have no points, then the node is empty
    if (points.size() == 0)
    {
        nodeType = BVHNodeType::Empty;
        totalMass = 0;
        centerOfMass = Vector3{0, 0, 0};
        averageNormal = Vector3{0, 0, 0};
        elementID = -1;
        numNodesInBranch = 1;
    }
    // If we have only one point, then the node is a leaf
    else if (points.size() == 1)
    {
        MassNormalPoint mp = points[0];
        nodeType = BVHNodeType::Leaf;
        totalMass = mp.mass;
        centerOfMass = mp.point;
        averageNormal = mp.normal;
        minCoords = mp.point;
        maxCoords = mp.point;
        elementID = mp.elementID;
        numNodesInBranch = 1;
    }
    // Otherwise, we need to recursively split and compute averages
    else
    {
        nodeType = BVHNodeType::Interior;
        elementID = -1;
        // Reserve space for splitting the points into lesser and greater
        int nPoints = points.size();
        std::vector<MassNormalPoint> lesserPoints;
        lesserPoints.reserve(nPoints / 2 + 1);
        std::vector<MassNormalPoint> greaterPoints;
        greaterPoints.reserve(nPoints / 2 + 1);

        // Compute the plane over which to split the points
        splitPoint = AxisSplittingPlane(points, axis);

        // Split the points over the median
        for (int i = 0; i < nPoints; i++)
        {
            double coord = GetCoordFromBody(points[i], axis);
            if (coord <= splitPoint)
                lesserPoints.push_back(points[i]);
            else
                greaterPoints.push_back(points[i]);
        }
        // Recursively construct children
        int nextAxis = NextAxis(axis);

        // If a root node was provided when constructing this, keep it;
        // otherwise, this must be the root, so use it.
        BVHNode6D *nextRoot = (root) ? root : this;
        // Add the children
        BVHNode6D *lesserNode = new BVHNode6D(lesserPoints, nextAxis, nextRoot);
        BVHNode6D *greaterNode = new BVHNode6D(greaterPoints, nextAxis, nextRoot);
        children.push_back(lesserNode);
        children.push_back(greaterNode);
        numNodesInBranch = lesserNode->numNodesInBranch + greaterNode->numNodesInBranch + 1;
        // Get the averages from children
        averageDataFromChildren();
    }
}

BVHNode6D::~BVHNode6D()
{
    for (size_t i = 0; i < children.size(); i++)
    {
        delete children[i];
    }
}

size_t BVHNode6D::assignIDsRecursively(size_t startID)
{
    nodeID = startID;
    size_t nextID = nodeID + 1;
    if (nodeType == BVHNodeType::Interior)
    {
        for (size_t i = 0; i < children.size(); i++)
        {
            nextID = children[i]->assignIDsRecursively(nextID);
        }
    }
    return nextID;
}

void BVHNode6D::addAllFaces(MeshPtr &mesh, std::vector<GCFace> &faces) {
    if (nodeType == BVHNodeType::Empty) {
        return;
    }
    else if (nodeType == BVHNodeType::Leaf) {
        faces.push_back(getSingleFace(mesh));
    }
    else {
        for (BVHNode6D* child : children) {
            child->addAllFaces(mesh, faces);
        }
    }
}

void BVHNode6D::printSummary()
{
    if (nodeType == BVHNodeType::Empty)
    {
        std::cout << "Empty node" << std::endl;
    }
    else if (nodeType == BVHNodeType::Leaf)
    {
        std::cout << "Leaf node (mass " << totalMass << ", center " << centerOfMass << ")" << std::endl;
    }
    else
    {
        std::cout << "Interior node (mass " << totalMass << ",\n  center " << centerOfMass
                  << ",\n  " << children.size() << " children)" << std::endl;

        for (BVHNode6D *child : children)
        {
            child->printSummary();
        }
    }
}

MassNormalPoint BVHNode6D::GetMassNormalPoint()
{
    return MassNormalPoint{totalMass, averageNormal, centerOfMass, elementID};
}

GCFace BVHNode6D::getSingleFace(MeshPtr &mesh)
{
    if (nodeType != BVHNodeType::Leaf)
    {
        std::cerr << "Tried to getSingleFace() from a non-leaf node" << std::endl;
        exit(1);
    }
    return mesh->face(elementID);
}

void BVHNode6D::averageDataFromChildren()
{
    totalMass = 0;
    centerOfMass = Vector3{0, 0, 0};
    averageNormal = Vector3{0, 0, 0};
    minCoords = children[0]->minCoords;
    maxCoords = children[0]->maxCoords;

    for (BVHNode6D *child : children)
    {
        totalMass += child->totalMass;
        centerOfMass += child->totalMass * child->centerOfMass;
        averageNormal += child->totalMass * child->averageNormal;
        minCoords = vectorMin(minCoords, child->minCoords);
        maxCoords = vectorMax(maxCoords, child->maxCoords);
    }

    centerOfMass /= totalMass;
    averageNormal = averageNormal.normalize();
}

double BVHNode6D::AxisSplittingPlane(std::vector<MassNormalPoint> &points, int axis)
{
    size_t nPoints = points.size();
    std::vector<double> coords(nPoints);

    for (size_t i = 0; i < nPoints; i++)
    {
        coords[i] = GetCoordFromBody(points[i], axis);
    }

    std::sort(coords.begin(), coords.end());

    size_t splitIndex = -1;
    double minWidths = INFINITY;

    for (size_t i = 0; i < nPoints; i++)
    {
        double width1 = coords[i] - coords[0];
        double width2 = (i == nPoints - 1) ? 0 : coords[nPoints - 1] - coords[i + 1];

        double sumSquares = width1 * width1 + width2 * width2;
        if (sumSquares < minWidths)
        {
            minWidths = sumSquares;
            splitIndex = i;
        }
    }

    double splitPoint = (coords[splitIndex] + coords[splitIndex + 1]) / 2;
    return splitPoint;
}

bool BVHNode6D::isAdmissibleFrom(Vector3 atPos)
{
    if (nodeType == BVHNodeType::Leaf) {
        if (centerOfMass == atPos) return false;
        else return true;
    }
    if (nodeType == BVHNodeType::Interior) {
        double d = norm(centerOfMass - atPos);
        return nodeRatio(d) < thresholdTheta;
    }
    else return true;
}

void BVHNode6D::recomputeCentersOfMass(MeshPtr &mesh, GeomPtr &geom)
{
    if (nodeType == BVHNodeType::Empty)
    {
        totalMass = 0;
        centerOfMass = Vector3{0, 0, 0};
    }
    // For a leaf, just set centers and bounds from the one body
    else if (nodeType == BVHNodeType::Leaf)
    {
        GCFace f = mesh->face(elementID);
        totalMass = geom->faceArea(f);
        centerOfMass = faceBarycenter(geom, f);
        minCoords = centerOfMass;
        maxCoords = centerOfMass;
    }
    else
    {
        // Recursively compute bounds for all children
        for (size_t i = 0; i < children.size(); i++)
        {
            children[i]->recomputeCentersOfMass(mesh, geom);
        }
        averageDataFromChildren();
    }
}

} // namespace rsurfaces
#pragma once
using namespace std;
#include <vector>
#include <tuple>

struct MeshVertex {
    WORD     index;
    XMFLOAT3 normal;
};

typedef tuple<int, int> Pair;
struct MeshPath {
    Pair path;
    int count;
    
    MeshPath() {
        count = 1;
    }
};

typedef vector<MeshPath>::iterator PathIterator;


// A class to represent a graph object
class Graph {

    vector<MeshVertex>  m_vertexList;
    vector<MeshPath>    m_pathList;

public:
    Graph() {
    }
    ~Graph() {
    }

    vector<Pair> DetectBorders(SimpleVertex vertices[], int numVertex, WORD indices[], int numIndex)
    {
        // init graph from mesh verteies
        vector<MeshVertex> vecMeshVtx;
        for (int i = 0; i < numVertex; i++) {
            MeshVertex meshVtx;
            meshVtx.index = i;
            meshVtx.normal = vertices[i].Normal;
            vecMeshVtx.push_back(meshVtx);
        }
        InitMesh(vecMeshVtx);

        // handle triangle paths
        for (int i = 0; i < numIndex; i += 3) {
            InsertPath(indices[i], indices[i + 1]);
            InsertPath(indices[i + 1], indices[i + 2]);
            InsertPath(indices[i + 2], indices[i]);
        }
        vector<Pair> pairList;
        DetectXYPlaneBorders(pairList);
        return pairList;
    }

protected:

    void InitMesh(vector<MeshVertex> vertexes) {
        m_vertexList.clear();
        m_vertexList = vertexes; 
    }

    void InsertPath(int i, int j) {
        Pair path(i, j);
        PathIterator iter = FindExistingPath(i, j);
        if (iter != m_pathList.end()) {
            // found, add count
            iter->count++;
        } else { 
            // create new path
            MeshPath p;
            p.path = path;
            p.count = 1;
            m_pathList.push_back(p);
        }
    }

    // paths with single traversal are borders
    void DetectXYPlaneBorders(vector<Pair>& pairList) {
        auto iter = m_pathList.begin();
        for (; iter != m_pathList.end(); iter = next(iter)) {
            if (iter->count == 1) {
                if (IsOnXYPlane(iter->path)) {
                    pairList.push_back(iter->path);
                }
            }
        }
    }

private:
    
    PathIterator FindExistingPath(int i, int j) {
        Pair path(i, j);
        Pair reversePath(j, i);

        auto iter = FindPath(path);
        if (iter == m_pathList.end()) {
            iter = FindPath(reversePath);
        }
        return iter;
    }

    PathIterator FindPath(Pair path) {
        auto iter = m_pathList.begin();
        for (; iter != m_pathList.end(); iter=next(iter)) {
            if (iter->path == path)
                break;
        }
        return iter;
    }

    //check if path is on XP plane 
    bool IsOnXYPlane(Pair path) {
        bool bFrontFace = false;

        int i = get<0>(path);
        int j = get<1>(path);

        if (NormalOnXYPlane(i) && NormalOnXYPlane(j)) {
            bFrontFace = true;
        }
        return bFrontFace;
    }

    bool NormalOnXYPlane(int index) {
        // get index's normal
        auto iter = m_vertexList.begin();
        for (; iter != m_vertexList.end(); iter = next(iter)) {
            if (iter->index == index) {
                break;
            }
        }
        if (iter == m_vertexList.end()) {
            return false;
        }

        // NdotX & NdotY
        XMFLOAT3 axisX = XMFLOAT3(1.0, 0.0, 0.0);
        XMFLOAT3 axisY = XMFLOAT3(0.0, 1.0, 0.0);
        XMVECTOR X = XMLoadFloat3(&axisX);
        XMVECTOR Y = XMLoadFloat3(&axisY);
        XMVECTOR N = XMLoadFloat3(&iter->normal);

        XMVECTOR NodtX = XMVector3Dot(N, X);
        XMVECTOR NodtY = XMVector3Dot(N, Y);

        float f1, f2;
        XMStoreFloat(&f1, NodtX);
        XMStoreFloat(&f2, NodtY);

        return (f1 == 0.f && f2 == 0.f);
    }
};
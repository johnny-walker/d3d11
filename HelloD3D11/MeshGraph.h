#pragma once
using namespace std;
#include <vector>
#include <tuple>
#include <Windows.h>
#include <DirectXMath.h>

struct MeshVertex {
    WORD     index;
    XMFLOAT3 normal;
    XMFLOAT3 position;
};

// count travrsal times of triangles' paths for given 3D mesh
typedef tuple<int, int> Pair;
struct MeshPath {
    Pair path;
    int count;
    
    MeshPath() {
        count = 1;
    }
};

typedef vector<MeshPath>::iterator PathIter;


// A class to represent 3D mesh as a graph object

class MeshGraph {

    vector<MeshVertex>  m_vertexList;
    vector<MeshPath>    m_pathList;

public:
    MeshGraph() {
    }

    ~MeshGraph() {
		m_vertexList.clear();
		m_pathList.clear();
    }

    vector<Pair> DetectBorders(SimpleVertex vertices[], int numVertex, WORD indices[], int numIndex)
    {
        // init graph from mesh verteies
        m_vertexList.clear();
		m_pathList.clear();
        for (int i = 0; i < numVertex; i++) {
            MeshVertex meshVtx;
            meshVtx.index = i;
            meshVtx.normal = vertices[i].Normal;
			meshVtx.position = vertices[i].Pos;
            m_vertexList.push_back(meshVtx);
        }

        // handle triangle paths
        for (int i = 0; i < numIndex; i += 3) {
			InsertPath(indices[i],   indices[i+1]);
			InsertPath(indices[i+1], indices[i+2]);
			InsertPath(indices[i+2], indices[i]);
        }

		//detect path, only traverse once
        vector<Pair> pairList;
        DetectXYPlaneBorders(pairList);
        return pairList;
    }	

protected:
    void InsertPath(int i, int j) {
        Pair path(i, j);
        PathIter iter = FindExistingPath(i, j);
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
				// paths which traverse only once are borders
                pairList.push_back(iter->path);
            }
        }
    }
   
    PathIter FindExistingPath(int i, int j) {
        auto iter = FindPath(i, j);
        if (iter == m_pathList.end()) {
            iter = FindPath(j, i);
        }
        return iter;
    }

    PathIter FindPath(int i, int j) {
		XMFLOAT3 p0 = m_vertexList[i].position;
		XMFLOAT3 p1 = m_vertexList[j].position;
		XMFLOAT3 p0N = m_vertexList[i].normal;
		XMFLOAT3 p1N = m_vertexList[j].normal;
        auto iter = m_pathList.begin();
		for (; iter != m_pathList.end(); iter=next(iter)) {

			int u = get<0>(iter->path);
			int v = get<1>(iter->path);
			XMFLOAT3 v0 = m_vertexList[u].position;
			XMFLOAT3 v1 = m_vertexList[v].position;
			XMFLOAT3 n0 = m_vertexList[u].normal;
			XMFLOAT3 n1 = m_vertexList[v].normal;

			if (v0.x==p0.x && v0.y==p0.y && v0.z==p0.z &&
				v1.x==p1.x && v1.y==p1.y && v1.z==p1.z &&
				n0.x==p0N.x && n0.y==p0N.y && n0.z==p0N.z &&
				n1.x==p1N.x && n1.y==p1N.y && n1.z==p1N.z)
			{
				// paths are identical due to positons and normals are all equal  
				break;
			}
        }
        return iter;
    }

private:

    //check if path is on XP plane 
    bool IsPathOnXYPlane(Pair path) {
        int i = get<0>(path);
        int j = get<1>(path);
        return (IsNormPerpXYPlane(i) && IsNormPerpXYPlane(j));
    }

	// check if normal is perpendicular to xy plane
    bool IsNormPerpXYPlane(int index) {
        // get index's normal
		bool found = false; 
        auto iter = m_vertexList.begin();
        for (; iter != m_vertexList.end(); iter = next(iter)) {
            if (iter->index == index) {
				found = true;
                break;
            }
        }
        if (!found) {
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
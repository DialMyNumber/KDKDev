// Modified by Jae-Ho Nah
// 2014.08
// 1) Support SOA (refer to macro SOA_ARRAY)
// 2) Store AABB per mesh for depth pre-sorting

// 2014.11
// 1) Add loading opacity/specular texture maps
// 2) Modify the destroy() fuction for complete memory deallocation
// 3) To do: Currently, only transparent/translucent objects are classified using the material's alpha value.
//    For more effective handling of transparent/translucent objects, we need to additionally consider texture's alpha values.
// 4) Store the texid & the mipmapping flag of a material to prevent the renderer from finding the texid during every frame




//-----------------------------------------------------------------------------
// Copyright (c) 2007 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#if !defined(MODEL_OBJ_H)
#define MODEL_OBJ_H

#include <string>
#include <cstdio>
#include <map>
#include <vector>

#define SOA_ARRAY 1 //0:AOS (array of structures), 1:SOA (structure of arrays)

#define BINARY_INPUT_OUTPUT 1

//cacluates the AABB of each mesh and sort all meshes using the calculated AABBs (for a front-to-back order)
//NOTE: if transparent meshs are included in the scene, they are alredady loacted in the back of the mesh list
//In this case, they are not further sorted
//#define SORT_AABB


//-----------------------------------------------------------------------------
// Alias|Wavefront OBJ file loader.
//
// This OBJ file loader contains the following restrictions:
// 1. Group information is ignored. Faces are grouped based on the material
//    that each face uses.
// 2. Object information is ignored. This loader will merge everything into a
//    single object.
// 3. The MTL file must be located in the same directory as the OBJ file. If
//    it isn't then the MTL file will fail to load and a default material is
//    used instead.
// 4. This loader triangulates all polygonal faces during importing.
//-----------------------------------------------------------------------------

class ModelOBJ
{
public:

    struct MapConfigs{
        unsigned int id;
        unsigned int target;
        unsigned char isMipmapped;
    };

    struct Material
    {
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float shininess;        // [0 = min shininess, 1 = max shininess]
        float alpha;            // [0 = fully transparent, 1 = fully opaque]

        std::string name;
        std::string colorMapFilename;
        std::string bumpMapFilename;

        //2014.11.06 Added by Jae-Ho Nah
        std::string specularMapFilename;
        std::string opacityMapFilename;

        int materialIndex;
        bool opaque;

        //2014.11.21 Added by Jae-Ho Nah
        MapConfigs texInfo;
    };


    struct Vertex
    {
        float position[3];
        float texCoord[2];
        float normal[3];
        float tangent[4];
        float bitangent[3];
    };

    struct Mesh
    {
        int startIndex;
        int triangleCount;
        const Material *pMaterial;

#ifdef SORT_AABB
        float min[3];
        float max[3];
#endif

    };
#ifdef SORT_AABB
    struct MeshCenter_Index
    {
        float center;
        unsigned int index;
    };
#endif
    ModelOBJ();
    ~ModelOBJ();

    void destroy();
    bool import(const char *pszFilename, bool rebuildNormals = false);
    void normalize(float scaleTo = 1.0f, bool center = true);
    void reverseWinding();

    // Getter methods.

    void getCenter(float &x, float &y, float &z) const;
    float getWidth() const;
    float getHeight() const;
    float getLength() const;
    float getRadius() const;

    const int *getIndexBuffer() const;
    int getIndexSize() const;

    const Material &getMaterial(int i) const;
    Material *getMaterial(int i);
    const Mesh &getMesh(int i) const;

    int getNumberOfIndices() const;
    int getNumberOfMaterials() const;
    int getNumberOfMeshes() const;
    int getNumberOfTriangles() const;
    int getNumberOfVertices() const;

    const std::string &getPath() const;

    const Vertex &getVertex(int i) const;
    const Vertex *getVertexBuffer() const;
    int getVertexSize() const;


#if SOA_ARRAY
    const float *getVertexBufferPosition() const;
    const float *getVertexBufferNormal() const;
    const float *getVertexBufferTexCoord() const;
    const float *getVertexBufferTangent() const;
    const float *getVertexBufferBitangent() const;
    void AOStoSOA();

#if 0
    const void getVertexPosition(int i, float* retValue) const;
    const void getVertexNormal(int i, float* retValue) const;
    const void getVertexTexCoord(int i, float* retValue) const;
#endif

#endif

#ifdef SORT_AABB
    //const struct MeshCenter_Index* getSortedMeshIndices(int axis) const;
    void getSortedMeshIndices(int axis, struct MeshCenter_Index** retValue);
#endif


    bool hasNormals() const;
    bool hasPositions() const;
    bool hasTangents() const;
    bool hasTextureCoords() const;

    std::string m_directoryPath;

#if BINARY_INPUT_OUTPUT
	void serialize(FILE* fp);
	void deserialize(FILE* fp);
	template<typename T> void write_binary_single(FILE *fp, T& single_reference);
	template<typename T> void write_binary_array(FILE *fp, T& array_reference, int size, int length);
	template<typename T> void write_binary_vector(FILE *fp, T& vector_reference, int size, int length);

	template<typename T> void read_binary_single(FILE *fp, T& single_reference);
	template<typename T> void read_binary_array(FILE *fp, T& array_reference, int size);
	template<typename T> void read_binary_vector(FILE *fp, std::vector<T>& vector_reference, int size);
#endif



private:
    void addTrianglePos(int index, int material,
        int v0, int v1, int v2);
    void addTrianglePosNormal(int index, int material,
        int v0, int v1, int v2,
        int vn0, int vn1, int vn2);
    void addTrianglePosTexCoord(int index, int material,
        int v0, int v1, int v2,
        int vt0, int vt1, int vt2);
    void addTrianglePosTexCoordNormal(int index, int material,
        int v0, int v1, int v2,
        int vt0, int vt1, int vt2,
        int vn0, int vn1, int vn2);
    int addVertex(int hash, const Vertex *pVertex);
    void bounds(float center[3], float &width, float &height,
        float &length, float &radius) const;
    void buildMeshes();
    void generateNormals();
    void generateTangents();
    void importGeometryFirstPass(FILE *pFile);
    void importGeometrySecondPass(FILE *pFile);
    bool importMaterials(const char *pszFilename);
    void scale(float scaleFactor, float offset[3]);

    bool m_hasPositions;
    bool m_hasTextureCoords;
    bool m_hasNormals;
    bool m_hasTangents;

    int m_numberOfVertexCoords;
    int m_numberOfTextureCoords;
    int m_numberOfNormals;
    int m_numberOfTriangles;
    int m_numberOfMaterials;
    int m_numberOfMeshes;

    float m_center[3];
    float m_width;
    float m_height;
    float m_length;
    float m_radius;



    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<Vertex> m_vertexBuffer;

#if SOA_ARRAY
    std::vector<float> m_vertexBuffer_position; //float3
    std::vector<float> m_vertexBuffer_texCoord; //glm::vec2
    std::vector<float> m_vertexBuffer_normal; //float3
    std::vector<float> m_vertexBuffer_tangent; //float4
    std::vector<float> m_vertexBuffer_bitangent; //float3

    bool m_SOAConverted;
#endif

#ifdef SORT_AABB
    std::vector<MeshCenter_Index>  m_meshIndices_x_axis_sorted;
    std::vector<MeshCenter_Index>  m_meshIndices_y_axis_sorted;
    std::vector<MeshCenter_Index>  m_meshIndices_z_axis_sorted;
#endif

    std::vector<int> m_indexBuffer;
    std::vector<int> m_attributeBuffer;
    std::vector<float> m_vertexCoords;
    std::vector<float> m_textureCoords;
    std::vector<float> m_normals;

    std::map<std::string, int> m_materialCache;
    std::map<int, std::vector<int> > m_vertexCache;
};

//-----------------------------------------------------------------------------

inline void ModelOBJ::getCenter(float &x, float &y, float &z) const
{ x = m_center[0]; y = m_center[1]; z = m_center[2]; }

inline float ModelOBJ::getWidth() const
{ return m_width; }

inline float ModelOBJ::getHeight() const
{ return m_height; }

inline float ModelOBJ::getLength() const
{ return m_length; }

inline float ModelOBJ::getRadius() const
{ return m_radius; }

inline const int *ModelOBJ::getIndexBuffer() const
{ return &m_indexBuffer[0]; }

inline int ModelOBJ::getIndexSize() const
{ return static_cast<int>(sizeof(int)); }

inline const ModelOBJ::Material &ModelOBJ::getMaterial(int i) const
{ return m_materials[i]; }

inline ModelOBJ::Material *ModelOBJ::getMaterial(int i)
{ return &m_materials[i]; }

inline const ModelOBJ::Mesh &ModelOBJ::getMesh(int i) const
{ return m_meshes[i]; }

inline int ModelOBJ::getNumberOfIndices() const
{ return m_numberOfTriangles * 3; }

inline int ModelOBJ::getNumberOfMaterials() const
{ return m_numberOfMaterials; }

inline int ModelOBJ::getNumberOfMeshes() const
{ return m_numberOfMeshes; }

inline int ModelOBJ::getNumberOfTriangles() const
{ return m_numberOfTriangles; }

inline int ModelOBJ::getNumberOfVertices() const
{
#if SOA_ARRAY
    if (m_SOAConverted)
        return static_cast<int>(m_vertexBuffer_position.size()/3);
    else
#endif
    return static_cast<int>(m_vertexBuffer.size());
}



inline const std::string &ModelOBJ::getPath() const
{ return m_directoryPath; }

inline const ModelOBJ::Vertex &ModelOBJ::getVertex(int i) const
{ return m_vertexBuffer[i]; }

inline const ModelOBJ::Vertex *ModelOBJ::getVertexBuffer() const
{ return &m_vertexBuffer[0]; }


#if SOA_ARRAY

inline const float *ModelOBJ::getVertexBufferPosition() const
{ return &m_vertexBuffer_position[0]; }

inline const float *ModelOBJ::getVertexBufferNormal() const
{ return &m_vertexBuffer_normal[0]; }

inline const float *ModelOBJ::getVertexBufferTexCoord() const
{ return &m_vertexBuffer_texCoord[0]; }

inline const float *ModelOBJ::getVertexBufferTangent() const
{ return &m_vertexBuffer_tangent[0]; }

inline const float *ModelOBJ::getVertexBufferBitangent() const
{ return &m_vertexBuffer_bitangent[0]; }

#if 0
inline const void ModelOBJ::getVertexPosition(int i, float* retValue) const
{
    retValue[0] = m_vertexBuffer_position[i*3];
    retValue[1] = m_vertexBuffer_position[i*3+1];
    retValue[2] = m_vertexBuffer_position[i*3+2];
}

inline const void ModelOBJ::getVertexNormal(int i, float* retValue) const
{
    retValue[0] = m_vertexBuffer_normal[i*3];
    retValue[1] = m_vertexBuffer_normal[i*3+1];
    retValue[2] = m_vertexBuffer_normal[i*3+2];
}

inline const void ModelOBJ::getVertexTexCoord(int i, float* retValue) const
{
    retValue[0] = m_vertexBuffer_texCoord[i*2];
    retValue[1] = m_vertexBuffer_texCoord[i*2+1];
}
#endif



#endif


#ifdef SORT_AABB

//inline const struct MeshCenter_Index* ModelOBJ::getSortedMeshIndices (int axis) const
inline void ModelOBJ::getSortedMeshIndices (int axis, struct MeshCenter_Index** retValue)
{

    switch (axis)
    {
    case 0:
        *retValue =  &m_meshIndices_x_axis_sorted[0];
        break;
    case 1:
        *retValue =  &m_meshIndices_y_axis_sorted[0];
        break;
    default:
    case 2:
        *retValue = &m_meshIndices_z_axis_sorted[0];
        break;
    }
}


#endif



inline int ModelOBJ::getVertexSize() const
{ return static_cast<int>(sizeof(Vertex)); }

inline bool ModelOBJ::hasNormals() const
{ return m_hasNormals; }

inline bool ModelOBJ::hasPositions() const
{ return m_hasPositions; }

inline bool ModelOBJ::hasTangents() const
{ return m_hasTangents; }

inline bool ModelOBJ::hasTextureCoords() const
{ return m_hasTextureCoords; }

#endif

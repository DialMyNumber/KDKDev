//SkinNNLS file loader
//Jae-Ho Nah, 2014/07


#if !defined(MODEL_SKINNNLS_H)
#define MODEL_SKINNNLS_H
#define MAX_FRAMES 100

#include <glm/glm.hpp>
#include "esUtil.h"
#include <cstring>
#include <string>
#include <vector>


#define BINARY_INPUT_OUTPUT 1

class cNNLS
{
public:
    cNNLS();
    bool loadFile(const char* filename, int userDisplacementCorrections=3);
    void destroy();

    const ESMatrix *getBoneAnimationBuffer(int frame) const;
    const unsigned int *getVertexWeightsIndexBuffer() const;
    const float *getVertexWeightsValueBuffer() const;
    const float *getDisplacementBasisBuffer(int axis) const;
    const float *getDisplacementCoordBuffer(int frame) const;
    const float *getNormalBasisBuffer(int axis) const;
    const float *getNormalCoordBuffer(int frame) const;
    const int getNumFrames() const;
    const int getNumBones() const;
    const int getNumVertices() const;
    const char* getObjectFileName() const;
    ~cNNLS();

//private:
    std::vector<ESMatrix> boneAnimation[MAX_FRAMES];
    std::vector<glm::uvec4> vertexWeightsIndices;
    std::vector<glm::vec4> vertexWeightsValues;
    std::vector<float> displacementBasis[3];
    std::vector<float> displacementCoords[MAX_FRAMES];
    std::vector<float> normalBasis[3];
    std::vector<float> normalCoords[MAX_FRAMES];

    int nFrames;
    int nBones;
    int nVertices;
    int nDisplacementCorrections;

    /*
    ESMatrix **boneAnimation;
    float *vertexWeights[4];
    float *displacementBasis[3];
    float **displacementCoords;
    float *normalBasis[3];
    float **normalCoords;
    */

    char objFilename[30];

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
};




inline const ESMatrix *cNNLS::getBoneAnimationBuffer(int frame) const
{ return &boneAnimation[frame][0]; }


inline const unsigned int *cNNLS::getVertexWeightsIndexBuffer() const
{ return &vertexWeightsIndices[0].x; }

inline const float *cNNLS::getVertexWeightsValueBuffer() const
{ return &vertexWeightsValues[0].x; }

inline const float *cNNLS::getDisplacementBasisBuffer(int axis) const
{ return &displacementBasis[axis][0]; }

inline const float *cNNLS::getDisplacementCoordBuffer(int frame) const
{ return &displacementCoords[frame][0]; }


inline const float *cNNLS::getNormalBasisBuffer(int axis) const
{ return &normalBasis[axis][0]; }

inline const float *cNNLS::getNormalCoordBuffer(int frame) const
{ return &normalCoords[frame][0]; }


inline const int cNNLS::getNumFrames() const
{
    return nFrames;
}

inline const int cNNLS::getNumBones() const
{
    return nBones;
}

inline const int cNNLS::getNumVertices() const
{
    return nVertices;
}

inline const char* cNNLS::getObjectFileName() const
{
    return objFilename;
}


#endif

#ifndef ESUTIL_H
#define ESUTIL_H

#ifdef WIN32
#define ESUTIL_API  __cdecl
#define ESCALLBACK  __cdecl
#else
#define ESUTIL_API
#define ESCALLBACK
#endif

#include <GLES3/gl3.h>

typedef struct
{
   GLfloat   m[4][4];
} ESMatrix;

//
/// \brief multiply matrix specified by result with a scaling matrix and return new matrix in result
/// \param result Specifies the input matrix.  Scaled matrix is returned in result.
/// \param sx, sy, sz Scale factors along the x, y and z axes respectively
//
void ESUTIL_API esScale ( ESMatrix *result, GLfloat sx, GLfloat sy, GLfloat sz );

//
/// \brief multiply matrix specified by result with a translation matrix and return new matrix in result
/// \param result Specifies the input matrix.  Translated matrix is returned in result.
/// \param tx, ty, tz Scale factors along the x, y and z axes respectively
//
void ESUTIL_API esTranslate ( ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz );

//
/// \brief multiply matrix specified by result with a rotation matrix and return new matrix in result
/// \param result Specifies the input matrix.  Rotated matrix is returned in result.
/// \param angle Specifies the angle of rotation, in degrees.
/// \param x, y, z Specify the x, y and z coordinates of a vector, respectively
//
void ESUTIL_API esRotate ( ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z );

//
// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  Both distances must be positive.
//
void ESUTIL_API esFrustum ( ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ );

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param fovy Field of view y angle in degrees
/// \param aspect Aspect ratio of screen
/// \param nearZ Near plane distance
/// \param farZ Far plane distance
//
void ESUTIL_API esPerspective ( ESMatrix *result, float fovy, float aspect, float nearZ, float farZ );

//
/// \brief multiply matrix specified by result with a perspective matrix and return new matrix in result
/// \param result Specifies the input matrix.  new matrix is returned in result.
/// \param left, right Coordinates for the left and right vertical clipping planes
/// \param bottom, top Coordinates for the bottom and top horizontal clipping planes
/// \param nearZ, farZ Distances to the near and far depth clipping planes.  These values are negative if plane is behind the viewer
//
void ESUTIL_API esOrtho ( ESMatrix *result, float left, float right, float bottom, float top, float nearZ, float farZ );

//
/// \brief perform the following operation - result matrix = srcA matrix * srcB matrix
/// \param result Returns multiplied matrix
/// \param srcA, srcB Input matrices to be multiplied
//
void ESUTIL_API esMatrixMultiply ( ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB );

//
//// \brief return an indentity matrix
//// \param result returns identity matrix
//
void ESUTIL_API esMatrixLoadIdentity ( ESMatrix *result );

//
/// \brief Generate a transformation matrix from eye position, look at and up vectors
/// \param result Returns transformation matrix
/// \param posX, posY, posZ           eye position
/// \param lookAtX, lookAtY, lookAtZ  look at vector
/// \param upX, upY, upZ              up vector
//
void ESUTIL_API
esMatrixLookAt ( ESMatrix *result,
                 float posX,    float posY,    float posZ,
                 float lookAtX, float lookAtY, float lookAtZ,
                 float upX,     float upY,     float upZ );

char* ESUTIL_API esShaderFileRead(const char *filename);
void ESUTIL_API esShaderFileFree(char *shaderSrc);
GLuint ESUTIL_API esLoadShader ( GLenum type, const char *shaderSrc );
GLuint ESUTIL_API esLoadShaderFromFile ( GLenum type, const char *filename );
GLuint ESUTIL_API esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc );
GLuint ESUTIL_API esLoadProgramTessellation ( const char *vertShaderSrc,
		const char *tessControlShaderSrc,
		const char *tessEvaluationShaderSrc,
		const char *fragShaderSrc ); //Added by Jae-Ho Nah

GLuint ESUTIL_API esLoadProgramFromFile ( const char *vertShaderFile, const char *fragShaderFile );
GLuint ESUTIL_API esLoadProgramFromFileTessellation ( const char *vertShaderFile,
		const char *tessControlShaderFile,
		const char *tessEvaluationShaderFile,
		const char *fragShaderFile );//Added by Jae-Ho Nah

GLuint ESUTIL_API esLinkProgram(GLuint programObject);

#endif

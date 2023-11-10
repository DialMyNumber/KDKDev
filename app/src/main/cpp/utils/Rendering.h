/****************************************************************************/
/* Copyright (c) 2011, Markus Billeter, Ola Olsson, Erik Sintorn and Ulf Assarsson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/****************************************************************************/
#ifndef _chag_utils_Rendering_h_
#define _chag_utils_Rendering_h_

//#include <linmath/float4x4.h>
//#include <linmath/float3.h>
#include <math.h>
#include <glm/glm.hpp>

const float g_pi = 3.1415926535897932384626433832795f;

inline glm::mat4x4 make_identity()
{
    glm::mat4x4 m =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    return m;
}

inline glm::mat3x3 make_identity3x3()
{
    glm::mat3x3 m =
    {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
    };
    return m;
}


inline const glm::mat4x4 make_translation(const glm::vec3 &pos)
{
    glm::mat4x4 m = make_identity();
    m[3] = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
    return m;
}


/**
 * The resulting matrix is created identically to gluPerspective()
 * and takes identical parameters.
 */
inline glm::mat4x4 perspectiveMatrix(float fov, float aspectRatio, float n, float f)
{
  glm::mat4x4 m = make_identity();// <glm::mat4x4>();
    m[3][3] = 0.0f;
    float b = -1.0f / (f-n);
    float cotanFOV = 1.0f / tanf(fov*(float)g_pi/360.f);
    m[0][0] = cotanFOV / aspectRatio;
    m[1][1] = cotanFOV;
    m[2][2] = (f+n)*b;
    m[2][3] = -1.0f;
    m[3][2] = 2.0f*n*f*b;
    return m;
}



/**
 */
inline glm::mat4x4 lookAt(const glm::vec3 &eyePosition, const glm::vec3 &lookAt, const glm::vec3 &desiredUp)
{

  glm::vec3 forward = glm::normalize(lookAt - eyePosition);
  glm::vec3 side = glm::cross(forward, desiredUp);
  side = glm::normalize(side);
  glm::vec3 up = glm::cross(side, forward);

  glm::mat4x4 m = make_identity();// <float4x4>();
  m[0][0] = side.x;
  m[1][0] = side.y;
  m[2][0] = side.z;

  m[0][1] = up.x;
  m[1][1] = up.y;
  m[2][1] = up.z;

  m[0][2] = -forward.x;
  m[1][2] = -forward.y;
  m[2][2] = -forward.z;

  return m * make_translation(-eyePosition);
}

inline const glm::mat4x4 make_matrix(const glm::mat3x3 &r, const glm::vec3 &pos)
{
    glm::mat4x4 m =
    {
        { r[0][0], r[0][1], r[0][2], 0.0f },
        { r[1][0], r[1][1], r[1][2], 0.0f },
        { r[2][0], r[2][1], r[2][2], 0.0f },
        { pos.x, pos.y, pos.z, 1.0f }
    };
    return m;
}


#endif // _chag_utils_Rendering_h_

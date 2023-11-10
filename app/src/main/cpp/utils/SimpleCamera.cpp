/****************************************************************************/
/* Copyright (c) 2011, Ola Olsson
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
#include "SimpleCamera.h"
#include <ctype.h>
#include <stdio.h>
#include "rendering.h"

#if 1
//#include <Windows.h>

// helper function to check if shift button is pressed.
static bool isShiftPressed()
{
    return 0;
    //  return (( GetKeyState( VK_LSHIFT   ) < 0 ) || ( GetKeyState( VK_RSHIFT   ) < 0 ));
}




//template<typename T>
//const T make_identity();
template <typename T>
const T make_rotation_x(float angle);
template <typename T>
const T make_rotation_y(float angle);
template <typename T>
const T make_rotation_z(float angle);
template <typename T, typename U>
const T make_rotation(const U& axis, float angle);
//template <typename T>
//const T make_matrix(const float *e);
template <typename T, typename U>
const T make_scale(const U& scale);



template <>
const glm::mat3x3 make_rotation_x<glm::mat3x3>(float angle)
{
    glm::mat3x3 result = make_identity3x3();

    float s = sinf(angle);
    float c = cosf(angle);

    result[1][1] = c;
    result[2][1] = -s;

    result[1][2] = s;
    result[2][2] = c;

    return result;
}



template <>
const glm::mat3x3 make_rotation_y<glm::mat3x3>(float angle)
{
    glm::mat3x3 result = make_identity3x3();

    float s = sinf(angle);
    float c = cosf(angle);

    result[0][0] = c;
    result[0][2] = -s;

    result[2][0] = s;
    result[2][2] = c;

    return result;
}



template <>
const glm::mat3x3 make_rotation_z<glm::mat3x3>(float angle)
{
    glm::mat3x3 result = make_identity3x3();

    float s = sinf(angle);
    float c = cosf(angle);

    result[0][0] = c;
    result[0][1] = -s;

    result[1][0] = s;
    result[1][1] = c;

    return result;
}


template <>
const glm::mat3x3 make_rotation<glm::mat3x3>(const glm::vec3& axis, float angleRad)
{
    glm::mat3x3 res;
    glm::vec3 v = glm::normalize(axis);

    float sinA = sinf(angleRad);
    float cosA = cosf(angleRad);
    float cosI = 1.0f - cosA;

    res[0][0] = (cosI * v.x * v.x) + (cosA);
    res[1][0] = (cosI * v.x * v.y) - (sinA * v.z);
    res[2][0] = (cosI * v.x * v.z) + (sinA * v.y);

    res[0][1] = (cosI * v.y * v.x) + (sinA * v.z);
    res[1][1] = (cosI * v.y * v.y) + (cosA);
    res[2][1] = (cosI * v.y * v.z) - (sinA * v.x);

    res[0][2] = (cosI * v.z * v.x) - (sinA * v.y);
    res[1][2] = (cosI * v.z * v.y) + (sinA * v.x);
    res[2][2] = (cosI * v.z * v.z) + (cosA);

    return res;
}

template <>
const glm::mat4x4 make_rotation_x<glm::mat4x4>(float angle)
{
    return make_matrix(make_rotation_x<glm::mat3x3>(angle), glm::vec3(0.0f, 0.0f, 0.0f));
}



template <>
const glm::mat4x4 make_rotation_y<glm::mat4x4>(float angle)
{
    return make_matrix(make_rotation_y<glm::mat3x3>(angle), glm::vec3(0.0f, 0.0f, 0.0f));
}



template <>
const glm::mat4x4 make_rotation_z<glm::mat4x4>(float angle)
{
    return make_matrix(make_rotation_z<glm::mat3x3>(angle), glm::vec3(0.0f, 0.0f, 0.0f));
}



template <>
const glm::mat4x4 make_rotation<glm::mat4x4>(const glm::vec3& axis, float angleRad)
{
    return make_matrix(make_rotation<glm::mat3x3>(axis, angleRad), glm::vec3(0.0f, 0.0f, 0.0f));
}


namespace chag
{





SimpleCamera::SimpleCamera()
{
    m_prevMousePosition = glm::uvec2(-1, -1);
    m_transform = glm::mat4x4();
    m_base = make_identity3x3();// make_identity<float3x3>();
  m_moveSpeed = 0.0f;
  m_strafeSpeed = 0.0f;
  m_rollSpeed = 0.0f;
  m_moveVel = 100.0f;
}



void SimpleCamera::init(const glm::vec3 &up, const glm::vec3 &fwd, const glm::vec3 &pos)
{
    m_base = glm::mat3x3(glm::cross(up, fwd), up, fwd);
  m_transform = make_matrix(m_base, pos);
}



const glm::vec3 SimpleCamera::getPosition() const
{
  return glm::vec3(m_transform[3]);
}



const glm::vec3 SimpleCamera::getDirection() const
{
  return glm::vec3(m_transform[2]);
}



const glm::vec3 SimpleCamera::getUp() const
{
 return glm::vec3(m_transform[1]);
}



void SimpleCamera::pitch(float angleRadians)
{
  m_transform = m_transform * make_rotation_x<glm::mat4x4>(angleRadians);
}



void SimpleCamera::roll(float angleRadians)
{
  m_transform = m_transform * make_rotation_z<glm::mat4x4>(angleRadians);
}



void SimpleCamera::yaw(float angleRadians)
{
  // this will cause a truly local yaw, which causes tumbling, what we
  // probably want is a yaw around the global up axis, in local space.
  //
  // -> m_transform = m_transform * make_rotation_y<float4x4>(angleRadians);

  // Instead yaw around world up axis.
  //float3x3 m = make_rotation_y<float3x3>(angleRadians) * make_matrix3x3(m_transform);
  
  //glm::mat3x3 m = make_rotation<glm::mat3x3>(m_base[1], angleRadians) * make_matrix3x3(m_transform);
    glm::mat3x3 m = make_rotation<glm::mat3x3>(m_base[1], angleRadians) * glm::mat3x3(m_transform);

  
  m_transform = make_matrix(m, getPosition());
}



void SimpleCamera::move(float distance)
{
    m_transform = m_transform * make_translation(glm::vec3(0.0f, 0.0f, distance));
}



void SimpleCamera::strafe(float distance)
{
  m_transform = m_transform * make_translation(glm::vec3(distance, 0.0f, 0.0f));
}



const glm::mat4x4 SimpleCamera::getTransform() const 
{ 
  return m_transform; 
}



void SimpleCamera::setTransform(const glm::mat4x4 &tfm) 
{ 
  m_transform = tfm; 
}



bool SimpleCamera::handleKeyInput(uint8_t keyCode, bool down)
{
  const float vel = down ? m_moveVel : 0.0f;

    switch(tolower(keyCode))
    {
  case 'w':
    m_moveSpeed = vel;
    break;
  case 's':
    m_moveSpeed = -vel;
    break;
  case 'a':
    m_strafeSpeed = vel;
    break;
  case 'd':
    m_strafeSpeed = -vel;
    break;
  case 'q':
    m_rollSpeed = down ? 1.0f : 0.0f;
    break;
  case 'e':
    m_rollSpeed = down ? -1.0f : 0.0f;
    break;
  default:
    return false;
  };
  return true;
}



void SimpleCamera::handleMouseInput(const glm::uvec2 &mousePosition)
{
  if (m_prevMousePosition.x > 0 && m_prevMousePosition.y > 0)
  {
    glm::uvec2 delta = mousePosition - m_prevMousePosition;

    pitch(-float(delta.y) / 100.0f);
    yaw(-float(delta.x) / 100.0f);
  }
  m_prevMousePosition = mousePosition;
}



void SimpleCamera::resetMouseInputPosition()
{
  m_prevMousePosition = glm::uvec2(-1, -1);
}



void SimpleCamera::update(float dt)
{
  const float speedBoost = isShiftPressed() ? 10.0f : 1.0f;
  move(speedBoost * m_moveSpeed * dt);
  strafe(speedBoost * m_strafeSpeed * dt);
  roll(speedBoost * m_rollSpeed * dt);
}


}; // namespace chag

#endif

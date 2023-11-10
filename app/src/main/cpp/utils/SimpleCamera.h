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
#ifndef _SimpleCamera_h_
#define _SimpleCamera_h_

#include "glm/glm.hpp"
/*
#include <linmath/glm::vec3.h>
#include <linmath/glm::mat4x4.h>
#include <linmath/glm::mat3x3.h>
#include <linmath/int2.h>
*/
namespace chag
{

/**
 * Simple camera updating class, that can handle yaw pitch and roll. Note that the yaw is performed around the WORLD up axis
 * which is more practical when flying over a ground plane (no rolling introduced).
 * 
 * Default Conventions:
 *  y is interpreted as up
 *  z as forwards (NOTE: NOT -z)
 * However as long as the getDirection/Position/Up functions are used (and not getTransform) you don't have to care.
 */
class SimpleCamera
{
public:
  SimpleCamera();
  void init(const glm::vec3 &up, const glm::vec3 &fwd, const glm::vec3 &pos = glm::vec3(0.0f, 0.0f, 0.0f));

  const glm::vec3 getPosition() const;
  const glm::vec3 getDirection() const;
  const glm::vec3 getUp() const;
  void pitch(float angleRadians);
  void roll(float angleRadians);
  void yaw(float angleRadians);
  void move(float distance);
  void strafe(float distance);

  const glm::mat4x4 getTransform() const;
  void setTransform(const glm::mat4x4 &tfm);

  /**
   * Uses the keys 
   *  fwd/back = 'w'/'s' 
   *  strafe L/R: 'a'/'d' 
   *  roll L/R: 'q' 'e'
   *  Internally sets movement velocities, which are then used whenever update() is called.
   *  conveniently use from glutKeyboardFunc/glutKeyboardUpFunc function.
   *  returns true if key was handled.
   */
  bool handleKeyInput(uint8_t keyCode, bool down);
  /**
   * Call from glutMotionFunc to make camera pitch and yaw.
   */
  void handleMouseInput(const glm::uvec2 &mousePosition);
  /**
   * call from, for example, glutMouseFunc, to ensure the camera doesnt leap when active movement happens.
   */
  void resetMouseInputPosition();
  /**
   * Applies the speeds for movement and roll to camera transform, using the delta time 'dt', handy to call from glutIdleFunc, for example.
   * Checks if the SHIFT key is down, in which case it increases the speeds 10 fold.
   */
  void update(float dt);
  /**
   * Sets the velocity used for moving and strafing.
   */
  void setMoveVelocity(float vel) { m_moveVel = vel; }

  const glm::vec3 getWorldUp() const { return m_base[1]; }

protected:
  glm::mat4x4 m_transform;
  glm::mat3x3 m_base;
  float m_moveVel;
  float m_moveSpeed;
  float m_strafeSpeed;
  float m_rollSpeed;
  glm::uvec2 m_prevMousePosition;
};



}; // namespace chag


#endif // _SimpleCamera_h_

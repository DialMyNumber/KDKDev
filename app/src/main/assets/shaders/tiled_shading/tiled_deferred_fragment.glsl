#version 300 es
precision highp float;
precision highp int;
/****************************************************************************/
/* Copyright (c) 2011, Ola Olsson, Ulf Assarsson
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
#include "globals.glsl"
#include "tiledShading.glsl"
#include "srgb.glsl"

uniform sampler2D diffuseTex;
uniform sampler2D specularShininessTex;
uniform sampler2D ambientTex;
uniform sampler2D normalTex;
uniform highp sampler2D depthTex;


out vec4 resultColor;

vec3 unProject(vec2 fragmentPos, float fragmentDepth)
{
  vec4 pt = inverseProjectionMatrix * vec4(fragmentPos.x * 2.0 - 1.0, fragmentPos.y * 2.0 - 1.0, 2.0 * fragmentDepth - 1.0, 1.0);
  return vec3(pt.x, pt.y, pt.z) / pt.w;
}


vec3 fetchPosition(vec2 p)
{
  vec2 fragmentPos = vec2(p.x * invFbSize.x, p.y * invFbSize.y);
  float d = texelFetch(depthTex, ivec2(p), 0).x;
  return unProject(fragmentPos, d);
}



void main()
{
  vec3 diffuse = texelFetch(diffuseTex, ivec2(gl_FragCoord.xy), 0).xyz;
  vec4 specularShininess = texelFetch(specularShininessTex, ivec2(gl_FragCoord.xy), 0); 
  vec3 position = fetchPosition(gl_FragCoord.xy); 
  vec3 normal = texelFetch(normalTex, ivec2(gl_FragCoord.xy), 0).xyz; 
  vec3 viewDir = -normalize(position);

  vec3 lighting = evalTiledLighting(diffuse, specularShininess.xyz, specularShininess.w, position, normal, viewDir);
  resultColor = vec4(toSrgb(lighting + texelFetch(ambientTex, ivec2(gl_FragCoord.xy), 0).xyz), 1.0);
  //resultColor = vec4(position.x/512.0, position.y/512.0, -position.z/4096.0, 1.0);//  * vec4(1.0, 1.0, 1.0, 1.0);
  //resultColor = vec4(-position.z/4096.0, -position.z/4096.0, -position.z/4096.0, 1.0);//  * vec4(1.0, 1.0, 1.0, 1.0);
  
}

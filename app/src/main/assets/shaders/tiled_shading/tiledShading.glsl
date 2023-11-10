#ifndef _TILED_SHADING_GLSL_
#define _TILED_SHADING_GLSL_
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
 
uniform highp isampler2D lightGridCountOffsets2;
uniform  highp sampler2D  lightPositionRange3;
uniform  highp sampler2D  lightColor3;
uniform  highp isampler2D  tileLightIndexListsTex;


vec3 doLight(vec3 position, vec3 normal, vec3 diffuse, vec3 specular, float shininess, vec3 viewDir, vec3 lightPos, vec3 lightColor, float range)
{
  vec3 lightDir = lightPos - position;
  float dist = length(lightDir);
  lightDir = normalize(lightDir);
  float inner = 0.0;

  float ndotL = max(dot(normal, lightDir),0.0);
  //float att = max(1.0 - max(0.0, (dist - inner) / (range - inner)), 0.0);
  float att = max(1.0 - max(0.0, dist / range), 0.0);

	vec3 fresnelSpec = specular + (vec3(1.0) - specular) * pow(clamp(1.0 + dot(-viewDir, normal), 0.0, 1.0), 5.0);
	vec3 h = normalize(lightDir + viewDir);

	float normalizationFactor = ((shininess + 2.0) / 8.0);

	vec3 spec = fresnelSpec * pow(max(0.0, dot(h, normal)), shininess) * normalizationFactor;

  //return vec3(dist/16384.0f, dist/16384.0f, dist/16384.0f);//position.x/16384.0f, position.y/16384.0f, position.z/16384.0f); //no
  //return vec3(position.x/16384.0f, position.y/16384.0f, position.z/16384.0f); /no
  //return vec3(lightPos.x/16384.0f, lightPos.y/16384.0f,lightPos.z/16384.0f); //no
  //return vec3(lightPos.x/32.0f, lightPos.y/32.0f,lightPos.z/32.0f);
  //return vec3(lightDir.x/32.0f, lightDir.y/32.0f,lightDir.z/32.0f);
  //return  ndotL * lightColor * (diffuse + spec);
  //return  att * vec3(1.0, 1.0, 1.0);//(diffuse + spec); //yes
  //return (dist-inner) / 16384.0f * vec3(1.0, 1.0, 1.0);//(diffuse + spec); //no
  //return (range-inner) / 16384.0f * vec3(1.0, 1.0, 1.0);//(diffuse + spec); //no
  //return min(1.0f, max(0.0, (dist - inner) / (range - inner)))/64.0f  * vec3(1.0, 1.0, 1.0);//(diffuse + spec); //mid
  return att * ndotL * lightColor * (diffuse + spec);
  
}


vec3 doLight(vec3 position, vec3 normal, vec3 diffuse, vec3 specular, float shininess, vec3 viewDir, int lightIndex)
{

  ivec2 index = ivec2(lightIndex%32, lightIndex/32);
  //vec4 lightPosRange = texelFetch(lightPositionRange2, lightIndex);
  vec4 lightPosRange = texelFetch(lightPositionRange3, index, 0);
  
  vec3 lightPos =  lightPosRange.xyz;
  float lightRange =  lightPosRange.w;
  
  vec3 lightColor = texelFetch(lightColor3, index, 0).xyz;
  return doLight(position, normal, diffuse, specular, shininess, viewDir, lightPos, lightColor, lightRange);
}


// computes tiled lighting for the current fragment, using the built in gl_FragCoord to determine the correct tile.
vec3 evalTiledLighting(in vec3 diffuse, in vec3 specular, in float shininess, in vec3 position, in vec3 normal, in vec3 viewDir)
{
  //ivec2 l = ivec2(int(gl_FragCoord.x) / LIGHT_GRID_TILE_DIM_X, int(gl_FragCoord.y) / LIGHT_GRID_TILE_DIM_Y);
  
  int x = int (floor(gl_FragCoord.x)/32.0);
  int y = int (floor(gl_FragCoord.y)/32.0);
  ivec2 l = ivec2(x, y);
  
  ivec4 countOffset = texelFetch(lightGridCountOffsets2, l, 0) ;
  int lightCount = countOffset.x;
  int lightOffset = countOffset.y;
 
  
  vec3 color = vec3(0.0, 0.0, 0.0);
  
  //codes for debugging
  
  //float fc = float(lightCount);
  //float fo = float(lightOffset);  
  //color = vec3(fc/32.0, fo/32768.0, 0.0);
  //color = vec3(fc/32768.0, 0.0, 0.0);
  //float flx = float(x);//gl_FragCoord.x / 32.0 ; //float(l.x);
  //float fly = float(y);//gl_FragCoord.y / 32.0; //float(l.y);
  //color = vec3(flx/80.0f, fly/45.0f, 0.0);
  //return color;
  
  for (int i = 0; i < lightCount; ++i)
  {
    int offset = lightOffset+i;
    int lightIndex = texelFetch(tileLightIndexListsTex, ivec2(offset%256, offset/256), 0).x; 
    color += doLight(position, normal, diffuse, specular, shininess, viewDir, lightIndex);
  }
  
  return color;
}

#endif // _TILED_SHADING_GLSL_

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
#ifndef _LightGrid_h_
#define _LightGrid_h_

#include "Config.h"
//#include <utils/IntTypes.h>
//#include <linmath/int2.h>
//#include <linmath/float4x4.h>
#include "Light.h"

//#include "glm/glm.hpp"
#include "glm/detail/type_vec2.hpp"
#include "glm/detail/type_mat4x4.hpp"

/**
 * This class contains the representation, and construction of the light grid used for tiled deferred shading.
 * The grid is stored in statically sized arrays, the size is determined by LIGHT_GRID_MAX_DIM_X and LIGHT_GRID_MAX_DIM_Y
 * (see Config.h).
 */
class LightGrid
{
public:
  struct ScreenRect
  {
    glm::uvec2 min;
    glm::uvec2 max;
  };
  typedef std::vector<ScreenRect> ScreenRects;

  LightGrid() {};
  /**
   * Call to build the grid structure.
   * The tile size and (screen) resolution must combine to be less than the maximum defined byt
   * LIGHT_GRID_MAX_DIM_X and LIGHT_GRID_MAX_DIM_Y. 
   *
   * The 'gridMinMaxZ' pointer is optional, if provided, the min and max range is used to prune lights.
   * The values are expected to be in view space.
   * It must have the dimensions LIGHT_GRID_MAX_DIM_X x LIGHT_GRID_MAX_DIM_Y.
   */
  void build(const glm::uvec2 tileSize, const glm::uvec2 resolution, const Lights &lights, 
    const glm::mat4x4 &modelView, const glm::mat4x4 &projection, float near, 
    const std::vector<glm::vec2> &gridMinMaxZ);

  /**
   * Access to grid wide properties, set at last call to 'build'.
   */
  const glm::uvec2 &getTileSize() const { return m_tileSize; }
  const glm::uvec2 &getGridDim() const { return m_gridDim; }
  uint32_t getMaxTileLightCount() const { return m_maxTileLightCount; }
  uint32_t getTotalTileLightIndexListLength() const { return uint32_t(m_tileLightIndexLists.size()); }

  /**
   * tile accessor functions, returns data for an individual tile.
   */
  int tileLightCount(uint32_t x, uint32_t y) const { return m_gridCounts[x + y * LIGHT_GRID_MAX_DIM_X]; }
  const int *tileLightIndexList(uint32_t x, uint32_t y) const { return &m_tileLightIndexLists[m_gridOffsets[x + y * LIGHT_GRID_MAX_DIM_X]]; }
  const glm::vec2 getTileMinMax(uint32_t x, uint32_t y) const { return m_minMaxGridValid ? m_gridMinMaxZ[x + y * m_gridDim.x] : glm::vec2(0.0f, 0.0f); }


  /** 
   * Grid data pointer accessors, used for uploading to GPU.
   */
  const int *tileDataPtr() const { return m_gridOffsets; }
  const int *tileCountsDataPtr() const { return m_gridCounts; }
  const int *tileLightIndexListsPtr() const { return &m_tileLightIndexLists[0]; }

  /**
   * Returns the list of lights, transformed to view space, that are present in the grid, 
   * i.e. produces an on screen rectangle. The light indices in the tile light lists index 
   * into this list.
   */
  const Lights &getViewSpaceLights() const { return m_viewSpaceLights; }

  /**
   * Indicated whether there are any lights visible, i.e. with on-screen rects.
   */
  bool empty() const { return m_screenRects.empty(); }
  /**
   * if true, then the contents of the min/max grid is valid, 
   * i.e. the pointer was not 0 last time build was called.
   */
  bool minMaxGridValid() const { return m_minMaxGridValid; }

protected:

  /**
   * Builds rects list (m_screenRects) and view space lights list (m_viewSpaceLights).
   */
  void buildRects(const glm::uvec2 resolution, const Lights &lights, const glm::mat4x4 &modelView, const glm::mat4x4 &projection, float near);
  
  glm::uvec2 m_tileSize;
  glm::uvec2 m_gridDim;

  std::vector<glm::vec2> m_gridMinMaxZ;
  int m_gridOffsets[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
  int m_gridCounts[LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y];
  std::vector<int> m_tileLightIndexLists;
  uint32_t m_maxTileLightCount;
  bool m_minMaxGridValid;

  Lights m_viewSpaceLights;
  ScreenRects m_screenRects;
};


#endif // _LightGrid_h_

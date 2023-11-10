
#include "tiled_shading.h"
#include "glm/glm.hpp"
#include <time.h>
#include <jni.h>
#include <android_native_app_glue.h>

#if 1
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

using namespace chag;

const char *g_renderMethodNames[RM_Max] =
{
    "TiledDeferred",
    "TiledForward",
    "Simple",
};
const char *g_tiledDeferredTextureUnitNames[TDTU_Max - TDTU_LightIndexData] =
{
    "tileLightIndexListsTex",
    "diffuseTex",
    "specularShininessTex",
    "normalTex",
    "ambientTex",
    "depthTex",

    "lightGridCountOffsets2",
    "lightPositionRange3",
    "lightColor3",
};


tiled_shading_app::tiled_shading_app(int width, int height, int num, float x, float y)
: Bench(width, height)
{
    myPossibleLights = num;
    xPos = x;
    yPos = y;
    //LogMessage ("POTT", "tiled shading constructor");

    memset (g_renderTargetTextures, 0, sizeof(g_renderTargetTextures));
#if NEW_OBJ_LOADER
    m_diffuseMaps = new TextureMaps();;
    m_opacityMaps = new TextureMaps();;
    m_specularMaps = new TextureMaps();;
    m_normalMaps = new TextureMaps();;
#endif
    mInitialized = false;

    //LogMessage ("POTT", "tiled_shading_mResPath: %s", mResPath.c_str());
}

 tiled_shading_app::~tiled_shading_app()
 {
     //Shutdown();
 }

bool tiled_shading_app::startup()
{
    //LogMessage ("POTT", "startup");
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_DITHER);


    //LogMessage ("POTT", "create fbo..");
    createFbos(mWidth, mHeight);
    //LogMessage ("POTT", "create shaders..");
    createShaders();

    //LogMessage ("POTT", "startup1");

    const float q3[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f,
    };

    const float vQuad[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    //quad vbo/vao generation
    // Create and bind a BO for vertex data
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);

    // copy data into the buffer object
    //glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), q3, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vQuad), vQuad, GL_STATIC_DRAW);

    // set up vertex attributes
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)* 2, 0);

    glVertexAttribPointer(BENCH_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void *)0);
    glEnableVertexAttribArray(BENCH_VERTEX_POSITION);
    glVertexAttribPointer(BENCH_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void *)(sizeof (GL_FLOAT) * 2));
    glEnableVertexAttribArray(BENCH_TEXCOORD);


    // At this point the VAO is set up with two vertex attributes
    // referencing the same buffer object, and another buffer object
    // as source for index data. We can now unbind the VAO, go do
    // something else, and bind it again later when we want to render
    // with it.

    glBindVertexArray(0);

    glGenTextures(1, &g_lightGridCountOffsetsTexture);
    // initial size is 1, because glTexBuffer failed if it was empty, we'll shovel in data later.
    glBindTexture(GL_TEXTURE_2D, g_lightGridCountOffsetsTexture);

    /*
    //Create a texture from the byte array
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG16I,           //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
        */

    /*
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGBA32F,         //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGBA,    //Interpret the texture data as luminance data
        GL_FLOAT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
    */

    /*
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG16UI,          //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_UNSIGNED_SHORT,
        //GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
     */

    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG32I,           //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_INT,
        g_gridBuffer3); //Use the texturePixels array as data


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    CHECK_GL_ERROR();

    glGenTextures(1, &g_lightPositionRangeTexture3);
    glBindTexture(GL_TEXTURE_2D, g_lightPositionRangeTexture3);
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGBA32F,         //Store the texture as rgb
        32,             //The texture is 4 pixels wide
        32,             //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGBA,    //Interpret the texture data as luminance data
        GL_FLOAT,//Read the data as an array of unsigned bytes
        g_lightPositionRangeBuffer3);   //Use the texturePixels array as data


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();


    glGenTextures(1, &g_lightColorTexture3);
    glBindTexture(GL_TEXTURE_2D, g_lightColorTexture3);
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGB16F,          //Store the texture as rgb
        32,             //The texture is 4 pixels wide
        32,             //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGB, //Interpret the texture data as luminance data
        GL_FLOAT,//Read the data as an array of unsigned bytes
        g_lightColorBuffer3);   //Use the texturePixels array as data


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();



    glGenTextures(1, &g_tileLightIndexListsTexture);
    memset(g_tileLightIndexListsBuffer, 0, sizeof(g_tileLightIndexListsBuffer));

    glBindTexture(GL_TEXTURE_2D, g_tileLightIndexListsTexture);
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_R32I,            //Store the texture as rgb
        256,                //The texture is 4 pixels wide
        256,                //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RED_INTEGER, //Interpret the texture data as luminance data
        GL_INT,//Read the data as an array of unsigned bytes
        g_tileLightIndexListsBuffer);   //Use the texturePixels array as data


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();


    g_shaderGlobalsGl.init(1, 0, GL_DYNAMIC_DRAW);
    //LogMessage ("POTT", "startup2");

    const char *res_root = android_fopen_get_res_path();
    std::string sceneDirectory(res_root);
    sceneDirectory.append("/scenes/crysponza");
    std::string sceneFileName(sceneDirectory);
    sceneFileName.append("/sponza.obj");



#if !NEW_OBJ_LOADER
    g_model = new OBJModel(this->mResPath);
    if (!g_model->load(sceneFileName))
    {
        fprintf(stderr, "The file: '%s' could not be loaded.\n", sceneFileName.c_str());
        return 1;
    }

#else
    g_model2 = new ModelOBJ();
    g_model2->destroy();
    if (!g_model2->import(sceneFileName.c_str()))
    {
        fprintf(stderr, "The file: '%s' could not be loaded.\n", sceneFileName.c_str());
        return 1;
    }

    init_buffers();

#endif

    //g_camera.init(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //basic camera
    //g_camera.init(glm::vec3(15.67f, 0.0f, -80.34f), glm::vec3(-0.945f, 0.324f, -0.037f), glm::vec3(0.416f, 0.908f, 0.052f)); //house
    g_camera.init(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-0.945f, 0.324f, -0.037f), glm::vec3(15.67f, 0.0f, -80.34f)); //sponza
                                                                                                                               //x: 플러스 앞, y: 플러스 위, z: 플러스 왼쪽

#if !NEW_OBJ_LOADER
    g_far = glm::length(g_model->getAabb().getHalfSize()) * 3.0f;
    g_near = g_far / 1000.0f;
//   g_lights = generateLights(g_model->getAabb(), myPossibleLights);

#else
    float scene_center[3];
    float scene_half_width = g_model2->getWidth()*0.5f;
    float scene_half_height = g_model2->getHeight()*0.5f;
    float scene_half_length = g_model2->getLength()*0.5f;
    g_model2->getCenter(scene_center[0], scene_center[1], scene_center[2]);

    g_far = glm::length(glm::vec3(scene_half_width *0.5f, scene_half_height*0.5f, scene_half_length)) * 3.0f;
    g_near = g_far / 1000.0f;
    Aabb aabb = make_aabb(glm::vec3(scene_center[0] - scene_half_width, scene_center[1] - scene_half_height, scene_center[2] - scene_half_length),
                          glm::vec3(scene_center[0] + scene_half_width, scene_center[1] + scene_half_height, scene_center[2] + scene_half_length));

    g_lights = generateLights(aabb, NUM_POSSIBLE_LIGHTS);
#endif

    //LogMessage ("POTT", "startup3");


    //g_glTimer = new GLTimerQuery;
    //g_appTimer.start();
    //glutMainLoop();

    return true;
}

void tiled_shading_app::SetLights(int num)
{
    if(myPossibleLights != num) {
        g_lights = generateLights(g_model->getAabb(), myPossibleLights);
    }
    myPossibleLights = num;
}

void tiled_shading_app::changeViews(float x, float y) {
    xPos = x;
    yPos = y;
    //LogMessage("View", "x: %f, y: %f", xPos, yPos);
}

void tiled_shading_app::Draw()
{
    //LogMessage ("POTT", "draw");
    //glPushAttrib(GL_ALL_ATTRIB_BITS);
    //LogMessage("Draw", "()");

    switch (g_renderMethod)
    {
    case RM_TiledDeferred:
    {
                            //LogMessage ("POTT", "draw1");
                             CHECK_GL_ERROR();
                             // 1. deferred render model.
                             CHECK_GL_ERROR();
                             glBindFramebuffer(GL_FRAMEBUFFER, g_deferredFbo);
                             //LogMessage ("POTT", "draw2");

                             CHECK_GL_ERROR();
                             glViewport(0, 0, mWidth, mHeight);
                             CHECK_GL_ERROR();
                             glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                             CHECK_GL_ERROR();
                             //LogMessage ("POTT", "draw3");
                             glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                             CHECK_GL_ERROR();

                             //LogMessage ("POTT", "draw4");
#if !NEW_OBJ_LOADER

                             g_deferredShader->begin(false);
                             g_model->render(OBJModel::RF_Opaque);
                             g_deferredShader->end();
                             //LogMessage ("POTT", "draw5");
                             g_deferredShader->begin(true);
                             g_model->render(OBJModel::RF_AlphaTested);
                             g_deferredShader->end();
                             //LogMessage ("POTT", "draw6");
#else
                             g_deferredShader->begin(false);
                             render_model(false);
                             g_deferredShader->end();
                             g_deferredShader->begin(true);
                             render_model(true);
                             g_deferredShader->end();
#endif
                             CHECK_GL_ERROR();


                             // 2. build grid
                             //LogMessage ("POTT", "draw7");
                             std::vector<glm::vec2> depthRanges;
                             if (g_enableDepthRangeTest)
                             {
                                 downSampleDepthBuffer(depthRanges);
                             }
                             //LogMessage ("POTT", "draw8");
                             g_lightGrid.build(
                                 glm::uvec2(LIGHT_GRID_TILE_DIM_X, LIGHT_GRID_TILE_DIM_Y),
                                 glm::uvec2(mWidth, mHeight),
                                 g_lights,
                                 modelView,
                                 projection,
                                 g_near,
                                 depthRanges
                                 );
                             //buildTimer.stop();
                             //LogMessage ("POTT", "draw9");

                             // 3. apply tiled deferred lighting
                             //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
                             if (mNumMSAASamples>1)
                             	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
                             else
                             	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
                             //glBindFramebuffer(GL_FRAMEBUFFER, 0);

                             glViewport(0, 0, mWidth, mHeight);
                             glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                             glClear(GL_COLOR_BUFFER_BIT | (g_enablePreZ ? 0 : GL_DEPTH_BUFFER_BIT));
                             //LogMessage ("POTT", "draw10");

                             renderTiledDeferred(g_lightGrid, projection);
                             //LogMessage ("POTT", "draw11");
    }
        break;
    case RM_TiledForward:
    {
                            // 0. Pre-Z pass
                            if (g_enablePreZ)
                            {
                                drawPreZPass();
                            }

                            // 1. build grid
                            //buildTimer.start();
                            std::vector<glm::vec2> depthRanges;
                            if (g_enableDepthRangeTest && g_enablePreZ)
                            {
                                downSampleDepthBuffer(depthRanges);
                            }

                            g_lightGrid.build(
                                glm::uvec2(LIGHT_GRID_TILE_DIM_X, LIGHT_GRID_TILE_DIM_Y),
                                glm::uvec2(mWidth, mHeight),
                                g_lights,
                                modelView,
                                projection,
                                g_near,
                                depthRanges
                                );
                            //buildTimer.stop();
                            CHECK_GL_ERROR();

                            // 2. Render scene using light info from grid.
                            bindLightGridConstants(g_lightGrid);
                            CHECK_GL_ERROR();
                            //if (g_numMsaaSamples != 0 || g_enablePreZ)
                            if (g_enablePreZ)
                            {
                                glBindFramebuffer(GL_FRAMEBUFFER,   g_forwardFbo);
                            }
                            else
                            {
                                if (mNumMSAASamples>1)
                                	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
                                else
                                	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

                                //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
                                //glBindFramebuffer(GL_FRAMEBUFFER, 0);
                            }

                            CHECK_GL_ERROR();
                            glViewport(0, 0, mWidth, mHeight);
                            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

                            // if pre-z is enabled, we don't want to re-clear the frame buffer.
                            glClear(GL_COLOR_BUFFER_BIT | (g_enablePreZ ? 0 : GL_DEPTH_BUFFER_BIT));

                            CHECK_GL_ERROR();

#if !NEW_OBJ_LOADER
                            g_tiledForwardShader->begin(false);
                            g_model->render(OBJModel::RF_Opaque);
                            g_tiledForwardShader->end();
                            g_tiledForwardShader->begin(true);
                            g_model->render(OBJModel::RF_AlphaTested);
                            g_tiledForwardShader->end();
#else
                            g_tiledForwardShader->begin(false);
                            render_model(false);
                            g_tiledForwardShader->end();
                            g_tiledForwardShader->begin(true);
                            render_model(true);
                            g_tiledForwardShader->end();
#endif


                            CHECK_GL_ERROR();

                            //if (g_numMsaaSamples != 0 || g_enablePreZ)
                            if (g_enablePreZ)
                            {
                                checkFBO(g_oldFBO);
                                //glBindFramebuffer(GL_FRAMEBUFFER, 0);
                                if (mNumMSAASamples>1)
                                	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
                                else
                                	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
                                //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

                                glBindFramebuffer(GL_READ_FRAMEBUFFER, g_forwardFbo);
                                CHECK_GL_ERROR();
                                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_oldFBO);
                                CHECK_GL_ERROR();
//                              checkFBO(g_forwardFbo);
//                              checkFBO(g_oldFBO);

                                glBlitFramebuffer(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                                CHECK_GL_ERROR();
//                              checkFBO(g_forwardFbo);
//                              checkFBO(g_oldFBO);

                                CHECK_GL_ERROR();
                                //glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                                //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                                //glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultFbo);
                                //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFbo);

                                if (mNumMSAASamples>1)
                                {
                                	glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultFboMSAA);
                                	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFboMSAA);
                                }
                                else
                                {
                                    glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultFbo);
                                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFbo);
                                }

                                CHECK_GL_ERROR();
                            }
    }
        break;
    case RM_Simple:
    {
                      //buildTimer.start();
                      //buildTimer.stop();

                      //glBindFramebuffer(GL_FRAMEBUFFER, 0);
                      //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
					  if (mNumMSAASamples>1)
						glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
					  else
						glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

                      CHECK_GL_ERROR();

                      glViewport(0, 0, mWidth, mHeight);
                      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                      CHECK_GL_ERROR();

#if !NEW_OBJ_LOADER
                      g_simpleShader->begin(false);
                      CHECK_GL_ERROR();
                      g_model->render(OBJModel::RF_Opaque);
                      CHECK_GL_ERROR();
                      g_simpleShader->end();
                      CHECK_GL_ERROR();
                      g_simpleShader->begin(true);
                      CHECK_GL_ERROR();
                      g_model->render(OBJModel::RF_AlphaTested);
                      g_simpleShader->end();
#else
                      g_simpleShader->begin(false);
                      CHECK_GL_ERROR();
                      render_model(false);
                      CHECK_GL_ERROR();
                      g_simpleShader->end();
                      CHECK_GL_ERROR();
                      g_simpleShader->begin(true);
                      CHECK_GL_ERROR();
                      render_model(true);
                      g_simpleShader->end();
#endif

    }
        break;
    };

    //glDisable(GL_DEPTH_TEST);
    //glPopAttrib();
    //glEnable(GL_DEPTH_TEST);
    //glDisable(GL_BLEND);

    //g_glTimer->stop();
    //printInfo(1000.0f / g_glTimer->getAvgMs(), float(buildTimer.getElapsedTime() * 1000.0));
    //glutSwapBuffers();
}
bool tiled_shading_app::Update()
{
    //LogMessage ("POTT", "update1");
    bool isInitial = false;
    if (!mInitialized) {
        startup();
        isInitial = true;
        mInitialized = true;
    }

    //LogMessage ("POTT", "update2");
    projection = perspectiveMatrix(g_fov, float(mWidth) / float(mHeight), g_near, g_far);

    float currentX = xPos;
    float currentY = yPos;
    float beforeX;
    float beforey;
    float difx = 0;
    float dify = 0;

    if(beforeX != currentY) {
        difx = beforeX - currentX;
    }
    if(beforey != currentY){
        dify = beforey - currentY;
    }
    //LogMessage("View", "x: %f, y: %f", difx, dify);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraDirection = g_camera.getDirection();
    glm::vec3 cameraPos = g_camera.getPosition();
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    //look = look * glm::vec3(x, y, 0);
    //modelView = lookAt(g_camera.getPosition(), g_camera.getPosition() + g_camera.getDirection(), g_camera.getUp());
    //cameraDirection = glm::normalize(front) + g_camera.getDirection();
    clock_t timer = clock();
    cameraDirection = cameraDirection + glm::vec3(0, sin(timer * 0.0000005) * 0.3, cos(timer * 0.0000005));//
    glm::vec3 look = g_camera.getPosition() + cameraDirection;
    modelView = lookAt(cameraPos, look, g_camera.getUp());
    updateShaderGlobals(modelView, projection, g_camera.getWorldUp(), mWidth, mHeight);

    beforeX = currentX;
    beforey = currentY;

    static float frame = 0;
    frame += 1;

    clock_t second = clock();
    static float fps = frame / second;

#if 0
    if (frame >= 500)
    {
        Shutdown();
        //printf("loading the next benchmark\n");
        return GL_TRUE;
    }
#endif


#if 0
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#else

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    //glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    static int accum = 0;
    static bool sign = true;
#if !NEW_OBJ_LOADER
    static float add_value = (g_model->getAabb().max.x - g_model->getAabb().min.x) *0.0005f;
#else
    static float add_value = g_model2->getRadius() *0.0005f;
#endif

    if (accum == 100) { sign = false; add_value = -add_value; }
    else if (accum == 0) { sign = true; add_value = -add_value; }
    accum = (sign) ? accum + 1 : accum - 1;
    //g_lights = generateLights(g_model->getAabb(), myPossibleLights);
    //float lightspeed = 0.01f;
    //glm::vec3 lightdirection(1.0f, 0.0f, 0.0f);
    for (unsigned int i = 0; i < g_lights.size(); ++i)
    {
        //LogMessage ("POTT", "light position [%d] (%f %f %f)", i, g_lights[i].position[0], g_lights[i].position[1], g_lights[i].position[2]);
        //LogMessage ("loop", "%d", g_lights.size());
        g_lights[i].position = glm::vec3(g_lights[i].position[0] + add_value, g_lights[i].position[1] + add_value, g_lights[i].position[2] + add_value);
        //g_lights[i].position += lightspeed * lightdirection;
    }
#endif


    return isInitial;
}

void tiled_shading_app::Shutdown() {

#if 0
    //LogMessage ("POTT", "shutdown");
    glDepthMask(GL_TRUE);
    glDisable(GL_DITHER);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
#endif
#if NEW_OBJ_LOADER
    if (m_diffuseMaps)
    {
        m_diffuseMaps->clearMaps();
        delete m_diffuseMaps;
    }

    if (m_opacityMaps)
    {
        m_opacityMaps->clearMaps();
        delete m_opacityMaps;
    }
    if (m_specularMaps)
    {
        m_specularMaps->clearMaps();
        delete m_specularMaps;
    }
    if (m_normalMaps)
    {
        m_normalMaps->clearMaps();
        delete m_normalMaps;
    }
#else
    if (g_model)
    {
        delete g_model;
        g_model = NULL;
    }

    for (int i = 0; i < DRTI_Max; ++i)
    {
        deleteTextureIfUsed(g_renderTargetTextures[i]);
    }

    // deferred render target
    if (g_deferredFbo)
    {
        glDeleteFramebuffers(1, &g_deferredFbo);
    }

    // forward shading render target
    if (g_forwardFbo)
    {
        glDeleteFramebuffers(1, &g_forwardFbo);
    }


    if (g_minMaxDepthFbo)
    {
        glDeleteFramebuffers(1, &g_minMaxDepthFbo);
    }


    deleteIfThere(g_simpleShader);
    deleteIfThere(g_deferredShader);
    deleteIfThere(g_tiledDeferredShader);
    deleteIfThere(g_tiledForwardShader);
    deleteIfThere(g_downSampleMinMaxShader);

    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &vbuffer);
    glDeleteTextures(1, &g_lightGridCountOffsetsTexture);
    glDeleteTextures(1, &g_lightPositionRangeTexture3);
    glDeleteTextures(1, &g_lightColorTexture3);
    glDeleteTextures(1, &g_tileLightIndexListsTexture);

#if 0
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glClearColor(0, 0, 0, 1);
    glClearDepthf(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //intialize texture binding
    for (int i = 0; i < 8; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
#endif
#endif

}



glm::vec3 tiled_shading_app::hueToRGB(float hue)
{
    const float s = hue * 6.0f;
    float r0 = glm::clamp(s - 4.0f, 0.0f, 1.0f);
    float g0 = glm::clamp(s - 0.0f, 0.0f, 1.0f);
    float b0 = glm::clamp(s - 2.0f, 0.0f, 1.0f);

    float r1 = glm::clamp(2.0f - s, 0.0f, 1.0f);
    float g1 = glm::clamp(4.0f - s, 0.0f, 1.0f);
    float b1 = glm::clamp(6.0f - s, 0.0f, 1.0f);

    // annoying that it wont quite vectorize...
    return glm::vec3(r0 + r1, g0 * g1, b0 * b1);
}


void tiled_shading_app::renderTiledDeferred(LightGrid &grid, const glm::mat4x4 &projectionMatrix)
{
    {
        glViewport(0, 0, mWidth, mHeight);
        // 참고 | 이 부분이 뷰포트, x, y에 각각 값을 넣은 만큼 검은 화면만 보임
        // ex) 0, 90 -> 아래쪽 90픽셀만큼 짤려서 나옴
        CHECK_GL_ERROR();

#if 0
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        bindLightGridConstants(grid);


        g_tiledDeferredShader->begin(false);

        glBindVertexArray(quad_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


        g_tiledDeferredShader->end();

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDepthMask(GL_TRUE);


#if 0
        glPopAttrib();
#endif
    }
}


void tiled_shading_app::updateShaderGlobals(const glm::mat4x4 &viewMatrix, const glm::mat4x4 &projectionMatrix, const glm::vec3 &worldUpDirection, int width, int height)
{
    g_shaderGlobals.viewMatrix = viewMatrix;
    g_shaderGlobals.viewProjectionMatrix = projectionMatrix * viewMatrix;
    g_shaderGlobals.inverseProjectionMatrix = glm::inverse(projectionMatrix);
    g_shaderGlobals.normalMatrix = glm::transpose(glm::inverse(viewMatrix));
    g_shaderGlobals.worldUpDirection = worldUpDirection;
    g_shaderGlobals.ambientGlobal = g_ambientLight;
    g_shaderGlobals.invFbSize = glm::vec2(1.0f / float(width), 1.0f / float(height));
    g_shaderGlobals.fbSize = glm::ivec2(width, height);

    // copy to Gl
    g_shaderGlobalsGl.copyFromHost(&g_shaderGlobals, 1);
    // bind buffer.
    g_shaderGlobalsGl.bindSlot(GL_UNIFORM_BUFFER, TDUBS_Globals);
}


void tiled_shading_app::drawPreZPass()
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &g_oldFBO);


    glBindFramebuffer(GL_FRAMEBUFFER, g_forwardFbo);

    glViewport(0, 0, mWidth, mHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_LEQUAL);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

#if !NEW_OBJ_LOADER
    g_simpleShader->begin(false);
    g_model->render(OBJModel::RF_Opaque);
    g_simpleShader->end();
    g_simpleShader->begin(true);
    g_model->render(OBJModel::RF_AlphaTested);
    g_simpleShader->end();
#else
    g_simpleShader->begin(false);
    render_model(false);
    g_simpleShader->end();
    g_simpleShader->begin(true);
    render_model(true);
    g_simpleShader->end();

#endif



    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}


/*
// 참고 | 라이트 개수 조절용
void Renderer::rebuildingLights(int nums) {
    myPossibleLights = nums;
}
*/      // 이대로 사용하면 duplicate symbols 오류임

std::vector<Light> tiled_shading_app::generateLights(const Aabb &aabb, int num)
{
    std::vector<Light> result;
    // divide volume equally amongst lights
    const float lightVol = aabb.getVolume() / float(num);
    // set radius to be the cube root of volume for a light
    const float lightRad = pow(lightVol, 1.0f / 3.0f);
    // and allow some overlap
    const float maxRad = lightRad;// * 2.0f;
    const float minRad = lightRad;// / 2.0f;

    srand(0); //use fixed random numbers

    for (int i = 0; i < num; ++i)
    {
        float rad = randomRange(minRad, maxRad);
        glm::vec3 col = hueToRGB(randomUnitFloat()) * randomRange(0.4f, 0.7f);
        //glm::vec3 pos = { randomRange(aabb.min.x + rad, aabb.max.x - rad), randomRange(aabb.min.y + rad, aabb.max.y - rad), randomRange(aabb.min.z + rad, aabb.max.z - rad) };
        const float ind = rad / 8.0f;
        //glm::vec3 pos = { randomRange(aabb.min.x + ind, aabb.max.x - ind), randomRange(aabb.min.y + ind, aabb.max.y - ind), randomRange(aabb.min.z + ind, aabb.max.z - ind) };

        glm::vec3 pos;
        pos.x = randomRange(aabb.min.x + ind, aabb.max.x - ind);
        pos.y = randomRange(aabb.min.y + ind, aabb.max.y - ind);
        pos.z = randomRange(aabb.min.z + ind, aabb.max.z - ind);


        Light l = { pos, col, rad };

        result.push_back(l);
    }
    return result;
}

#if 0
static void onGlutKeyboard(unsigned char key, int, int)
{
    g_camera.setMoveVelocity(length(g_model->getAabb().getHalfSize()) / 10.0f);

    if (g_camera.handleKeyInput(key, true))
    {
        return;
    }

    switch (tolower(key))
    {
    case 27:
        glutLeaveMainLoop();
        break;
    case 'r':
        g_lights = generateLights(g_model->getAabb(), int(g_lights.size()));
        break;
    case 't':
        g_showGBuffer = (g_showGBuffer + 1) % (DRTI_Depth + 1);
        break;
    case 'l':
        createShaders();
        break;
    case 'p':
        g_showLights = !g_showLights;
        break;
    case 'g':
        g_showLightGrid = !g_showLightGrid;
        break;
    case 'm':
        g_renderMethod = RenderMethod((g_renderMethod + 1) % RM_Max);
        break;
    case 'c':
    {
                if (g_numMsaaSamples == 0)
                {
                    g_numMsaaSamples = 2;
                }
                else
                {
                    g_numMsaaSamples <<= 1;
                }
                if (g_numMsaaSamples > MAX_ALLOWED_MSAA_SAMPLES)
                {
                    g_numMsaaSamples = 0;
                }
                // now we must recompile shaders, and re-create FBOs
                createShaders();
                createFbos(mWidth, mHeight);
    }
        break;
    case 'u':
    {
                glm::vec3 pos = g_camera.getPosition();
                static int upAxis = 0;
                static glm::vec3 ups[] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
                static glm::vec3 fwds[] = { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } };
                g_camera.init(ups[upAxis], fwds[upAxis], pos);
                upAxis = (upAxis + 1) % 3;
    }
    case 'o':
        g_enablePreZ = !g_enablePreZ;
        break;
    case 'z':
        g_enableDepthRangeTest = !g_enableDepthRangeTest;
        break;
    case '+':
    {
                int newCount = min(int(NUM_POSSIBLE_LIGHTS), int(g_lights.size()) + NUM_POSSIBLE_LIGHTS / 8);
                g_lights = generateLights(g_model->getAabb(), newCount);
    }
        break;
    case '-':
    {
                int newCount = max(0, int(g_lights.size()) - NUM_POSSIBLE_LIGHTS / 8);
                g_lights = generateLights(g_model->getAabb(), newCount);
    }
        break;
     }
}



static void onGlutKeyboardUp(unsigned char _key, int, int)
{
    if (g_camera.handleKeyInput(_key, false))
    {
        return;
    }
}



static void onGlutSpecial(int key, int, int)
{
    if (key == GLUT_KEY_F1)
    {
        g_showInfo = !g_showInfo;
    }
}



static void onGlutMouse(int, int, int, int)
{
    g_camera.resetMouseInputPosition();
}



static void onGlutMotion(int x, int y)
{
    g_camera.handleMouseInput(glm::ivec2(x, y));
}



static void onGlutReshape(int width, int height)
{
    if (mWidth != width || mHeight != height)
    {
        createFbos(width, height);
    }
    mWidth = width;
    mHeight = height;
}
#endif







// helper function to create and attach a frame buffer target object.
GLuint tiled_shading_app::attachTargetTextureToFBO(GLuint fbo, GLenum attachment, int width, int height, GLenum internalFormat, GLenum format, GLenum type, int msaaSamples)
{

    GLuint targetTexture;
    glGenTextures(1, &targetTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    CHECK_GL_ERROR();


    if (msaaSamples == 0)
    {
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
        //glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

        CHECK_GL_ERROR();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, targetTexture, 0);
        CHECK_GL_ERROR();

        //glBindTexture(GL_TEXTURE_2D, 0);
    }





    return targetTexture;
}




void tiled_shading_app::checkFBO(uint32_t fbo)
{

	//LogMessage ("POTT", "before binding framebuffer", "");


    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //LogMessage ("POTT", "bind framebuffer error", "");
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        	LogMessage ("POTT", "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", "");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        	LogMessage ("POTT", "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", "");
            break;
#if 0
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            printf("FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            printf("FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
            break;
#endif
        case GL_FRAMEBUFFER_UNSUPPORTED:
        	LogMessage ("POTT", "FRAMEBUFFER_UNSUPPORTED\n", "");
            break;
        default:
        	LogMessage ("POTT", "Unknown framebuffer problem","%d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            break;
        }
        LogMessage ("POTT", "Error: bad frame buffer config", "");
        exit(-1);
    }

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
    if (mNumMSAASamples>1)
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
    else
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

}



void tiled_shading_app::deleteTextureIfUsed(GLuint texId)
{
    if (texId != 0)
    {
        glDeleteTextures(1, &texId);
    }
}




void tiled_shading_app::createFbos(int width, int height)
{
    int maxSamples = 0;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    //LogMessage ("POTT", "create fbo(1)");

    for (int i = 0; i < DRTI_Max; ++i)
    {
        deleteTextureIfUsed(g_renderTargetTextures[i]);
    }

    //LogMessage ("POTT", "create fbo(2)");
    // deferred render target
    if (!g_deferredFbo)
    {
        // only create if not already created.
        glGenFramebuffers(1, &g_deferredFbo);
    }

    //LogMessage ("POTT", "create fbo(3)");
    bool colorBufferFloatSupported = false;
    const GLubyte* pExtensions = glGetString(GL_EXTENSIONS);
    colorBufferFloatSupported = strstr((char*)pExtensions, "GL_EXT_color_buffer_half_float") != NULL;
    //LogMessage ("POTT", "color buffer half float supported","%d", colorBufferFloatSupported);


#if 1
    //LogMessage ("POTT", "create fbo(4)");
    glBindFramebuffer(GL_FRAMEBUFFER, g_deferredFbo);
    for (int i = 0; i < DRTI_Depth; ++i)
    {
        //LogMessage ("POTT", "fbo no. %d (%d, %d)", i, width, height);
        //g_renderTargetTextures[i] = attachTargetTextureToFBO(g_deferredFbo, GL_COLOR_ATTACHMENT0 + i, width, height, g_rgbaFpFormat, GL_RGBA, GL_FLOAT, g_numMsaaSamples);
        //g_renderTargetTextures[i] = attachTargetTextureToFBO(g_deferredFbo, GL_COLOR_ATTACHMENT0 + i, width, height, g_rgbaFpFormat, GL_RGBA, GL_FLOAT, g_numMsaaSamples);
        g_renderTargetTextures[i] = attachTargetTextureToFBO(g_deferredFbo, GL_COLOR_ATTACHMENT0 + i, width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, g_numMsaaSamples);

        //GLuint tiled_shading_app::attachTargetTextureToFBO(GLuint fbo, GLenum attachment, int width, int height, GLenum internalFormat, GLenum format, GLenum type, int msaaSamples)
    }
    //LogMessage ("POTT", "fbo depth");
    //g_renderTargetTextures[DRTI_Depth] = attachTargetTextureToFBO(g_deferredFbo, GL_DEPTH_ATTACHMENT, width, height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, g_numMsaaSamples);
    //g_renderTargetTextures[DRTI_Depth] = attachTargetTextureToFBO(g_deferredFbo, GL_DEPTH_ATTACHMENT, width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, g_numMsaaSamples);
    g_renderTargetTextures[DRTI_Depth] = attachTargetTextureToFBO(g_deferredFbo, GL_DEPTH_ATTACHMENT, width, height, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, g_numMsaaSamples);
#endif



    /*
     GLuint fbo_textures[4];
     glBindFramebuffer(GL_FRAMEBUFFER, g_deferredFbo);
     glGenTextures(4, fbo_textures);

     glBindTexture(GL_TEXTURE_2D, fbo_textures[0]);
     glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

     glBindTexture(GL_TEXTURE_2D, fbo_textures[1]);
     glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


     glBindTexture(GL_TEXTURE_2D, fbo_textures[2]);
     glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, mWidth, mHeight);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



     glBindTexture(GL_TEXTURE_2D, fbo_textures[3]);
     glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, mWidth, mHeight);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_textures[0], 0);
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo_textures[1], 0);
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fbo_textures[2], 0);
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo_textures[3], 0);

     */


    //LogMessage ("POTT", "create fbo(5)");
    //LogMessage ("POTT", "bind framebuffer error for attachment", "");
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        	LogMessage ("POTT", "FRAMEBUFFER_INCOMPLETE_ATTACHMENT", "");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        	LogMessage ("POTT", "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", "");
            break;
#if 0
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            printf("FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            printf("FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
            break;
#endif
        case GL_FRAMEBUFFER_UNSUPPORTED:
        	LogMessage ("POTT", "FRAMEBUFFER_UNSUPPORTED\n", "");
            break;
        default:
        	LogMessage ("POTT", "Unknown framebuffer problem","%d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            break;
        }
        LogMessage ("POTT", "Error: bad frame buffer config", "");
        exit(-1);
    }

    CHECK_GL_ERROR();
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(sizeof(bufs) / sizeof(bufs[0]), bufs);
    glReadBuffer(GL_NONE);


    //LogMessage ("POTT", "create fbo(6)");

    checkFBO(g_deferredFbo);

    /*
    LogMessage ("POTT", "create fbo(6.5)");
       // forward shading render target
    if (!g_forwardFbo)
    {
        // only create if not already created.
        glGenFramebuffers(1, &g_forwardFbo);
    }

    LogMessage ("POTT", "create fbo(7)");
    glBindFramebuffer(GL_FRAMEBUFFER, g_forwardFbo);
    g_forwardTargetTexture = attachTargetTextureToFBO(g_forwardFbo, GL_COLOR_ATTACHMENT0, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, g_numMsaaSamples);


    // Shared with deferred
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_renderTargetTextures[DRTI_Depth], 0);




    LogMessage ("POTT", "create fbo(8)");
    GLenum bufs1[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(sizeof(bufs1) / sizeof(bufs1[0]), bufs1);

    //glDrawBuffers(GL_COLOR_ATTACHMENT0,);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    checkFBO(g_forwardFbo);

*/

    GLenum bufs1[] = { GL_COLOR_ATTACHMENT0 };
    //LogMessage ("POTT", "create fbo(9)");
    if (!g_minMaxDepthFbo)
    {
        // only create if not already created.
        glGenFramebuffers(1, &g_minMaxDepthFbo);
    }

    //LogMessage ("POTT", "create fbo(10)");
    glBindFramebuffer(GL_FRAMEBUFFER, g_minMaxDepthFbo);

    glm::uvec2 tileSize = glm::uvec2(LIGHT_GRID_TILE_DIM_X, LIGHT_GRID_TILE_DIM_Y);
    glm::uvec2 resolution = glm::uvec2(width, height);
    glm::uvec2 gridRes = (resolution + tileSize - glm::uvec2(1, 1)) / tileSize;

    //g_minMaxDepthTargetTexture = attachTargetTextureToFBO(g_minMaxDepthFbo, GL_COLOR_ATTACHMENT0, gridRes.x, gridRes.y, GL_RG32F, GL_RGBA, GL_FLOAT, 0);
    //g_minMaxDepthTargetTexture = attachTargetTextureToFBO(g_minMaxDepthFbo, GL_COLOR_ATTACHMENT0, gridRes.x, gridRes.y, GL_RG32F, GL_RG, GL_FLOAT, 0);
    g_minMaxDepthTargetTexture = attachTargetTextureToFBO(g_minMaxDepthFbo, GL_COLOR_ATTACHMENT0, gridRes.x, gridRes.y, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 0);

    //LogMessage ("POTT", "create fbo(11)");
    glDrawBuffers(sizeof(bufs1) / sizeof(bufs1[0]), bufs1);
    //glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    checkFBO(g_minMaxDepthFbo);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
    //glBindFramebuffer(GL_FRAMEBUFFER, g_oldFBO);
    if (mNumMSAASamples>1)
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
    else
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

    //LogMessage ("POTT", "create fbo(12)");

}


void tiled_shading_app::bindObjModelAttributes(ComboShader *shader)
{
    CHECK_GL_ERROR();
    shader->bindAttribLocation(OBJModel::AA_Position, "position");
    shader->bindAttribLocation(OBJModel::AA_Normal, "normalIn");
    shader->bindAttribLocation(OBJModel::AA_TexCoord, "texCoordIn");
    shader->bindAttribLocation(OBJModel::AA_Tangent, "tangentIn");
    shader->bindAttribLocation(OBJModel::AA_Bitangent, "bitangentIn");
}

void tiled_shading_app::setObjModelUniformBindings(ComboShader *shader)
{
    CHECK_GL_ERROR();
    LogMessage ("POTT", "begin shader");
    shader->begin(false);

    LogMessage ("POTT", "set uniform");

    shader->setUniform("diffuse_texture", OBJModel::TU_Diffuse);
    shader->setUniform("opacity_texture", OBJModel::TU_Opacity);
    shader->setUniform("specular_texture", OBJModel::TU_Specular);
    shader->setUniform("normal_texture", OBJModel::TU_Normal);
    LogMessage ("POTT", "end shader");
    shader->end();
    LogMessage ("POTT", "buffer slot material");
    shader->setUniformBufferSlot("MaterialProperties", OBJModel::UBS_MaterialProperties);
    LogMessage ("POTT", "buffer slot global");

    shader->setUniformBufferSlot("Globals", TDUBS_Globals);
    LogMessage ("POTT", "end bindings");
}


void tiled_shading_app::setTiledLightingUniformBindings(ComboShader *shader)
{
    shader->setUniformBufferSlot("Globals", TDUBS_Globals);



    shader->begin(false);

    for (int i = TDTU_LightIndexData; i < TDTU_Max; ++i)
    {
        shader->setUniform(g_tiledDeferredTextureUnitNames[i - TDTU_LightIndexData], i);
    }
    shader->end();
}


template <typename T>
void tiled_shading_app::deleteIfThere(T *&shader)
{
    if (shader)
    {
        delete shader;
        shader = 0;
    }
}


void tiled_shading_app::createShaders()
{
    //LogMessage ("POTT", "createshaders(0)");
    SimpleShader::Context shaderCtx;
    shaderCtx.setPreprocDef("NUM_POSSIBLE_LIGHTS", NUM_POSSIBLE_LIGHTS);
    shaderCtx.setPreprocDef("LIGHT_GRID_TILE_DIM_X", LIGHT_GRID_TILE_DIM_X);
    shaderCtx.setPreprocDef("LIGHT_GRID_TILE_DIM_Y", LIGHT_GRID_TILE_DIM_Y);
    shaderCtx.setPreprocDef("LIGHT_GRID_MAX_DIM_X", LIGHT_GRID_MAX_DIM_X);
    shaderCtx.setPreprocDef("LIGHT_GRID_MAX_DIM_Y", LIGHT_GRID_MAX_DIM_Y);
    shaderCtx.setPreprocDef("NUM_MSAA_SAMPLES", int(g_numMsaaSamples));


    deleteIfThere(g_simpleShader);
    //LogMessage ("POTT", "createshaders(1)");
    g_simpleShader = new ComboShader("shaders/tiled_shading/simple_vertex.glsl", "shaders/tiled_shading/simple_fragment.glsl", shaderCtx, this->mResPath);
    CHECK_GL_ERROR();
    bindObjModelAttributes(g_simpleShader);
    //LogMessage ("POTT", "createshaders(2)");
    //g_simpleShader->bindFragDataLocation(0, "resultColor");
    CHECK_GL_ERROR();
    //LogMessage ("POTT", "createshaders(2.1)");
    g_simpleShader->link();
    //LogMessage ("POTT", "createshaders(2.2)");

    CHECK_GL_ERROR();
    //LogMessage ("POTT", "createshaders(2.3)");
    setObjModelUniformBindings(g_simpleShader);
    //LogMessage ("POTT", "createshaders(3)");

    // deferred shader
    deleteIfThere(g_deferredShader);
    g_deferredShader = new ComboShader("shaders/tiled_shading/deferred_vertex.glsl", "shaders/tiled_shading/deferred_fragment.glsl", shaderCtx, this->mResPath);
    bindObjModelAttributes(g_deferredShader);

    //g_deferredShader->bindFragDataLocation(DRTI_Diffuse, "outDiffuse");
    //g_deferredShader->bindFragDataLocation(DRTI_SpecularShininess, "outSpecularShininess");
    //g_deferredShader->bindFragDataLocation(DRTI_Normal, "outNormal");
    //g_deferredShader->bindFragDataLocation(DRTI_Ambient, "outAmbient");
    g_deferredShader->link();
    setObjModelUniformBindings(g_deferredShader);

    //LogMessage ("POTT", "createshaders(4)");
    // tiled deferred shader
    deleteIfThere(g_tiledDeferredShader);
    g_tiledDeferredShader = new ComboShader("shaders/tiled_shading/tiled_deferred_vertex.glsl", "shaders/tiled_shading/tiled_deferred_fragment.glsl", shaderCtx, this->mResPath);
    //g_tiledDeferredShader->bindFragDataLocation(0, "resultColor");
    g_tiledDeferredShader->link();
    setTiledLightingUniformBindings(g_tiledDeferredShader);

    //LogMessage ("POTT", "createshaders(5)");
    // tiled forward shader
    deleteIfThere(g_tiledForwardShader);
    g_tiledForwardShader = new ComboShader("shaders/tiled_shading/tiled_forward_vertex.glsl", "shaders/tiled_shading/tiled_forward_fragment.glsl", shaderCtx, this->mResPath);
    bindObjModelAttributes(g_tiledForwardShader);
    //g_tiledForwardShader->bindFragDataLocation(0, "resultColor");
    g_tiledForwardShader->link();
    setObjModelUniformBindings(g_tiledForwardShader);
    setTiledLightingUniformBindings(g_tiledForwardShader);

    //LogMessage ("POTT", "createshaders(6)");
    // downsample min/max shader
    deleteIfThere(g_downSampleMinMaxShader);
    g_downSampleMinMaxShader = new SimpleShader("shaders/tiled_shading/tiled_deferred_vertex.glsl", "shaders/tiled_shading/downsample_minmax_fragment.glsl", shaderCtx, this->mResPath);
    //g_downSampleMinMaxShader->bindFragDataLocation(0, "resultMinMax");
    g_downSampleMinMaxShader->link();
    g_downSampleMinMaxShader->setUniformBufferSlot("Globals", TDUBS_Globals);
    //LogMessage ("POTT", "createshaders(7)");
}


// helper to bind texture...
void tiled_shading_app::bindTexture(GLenum type, int texUnit, int textureId)
{
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(type, textureId);
}


void tiled_shading_app::bindLightGridConstants(const LightGrid &lightGrid)
{
    // pack grid data in int4 because this will work on AMD GPUs, where constant registers are 4-vectors.
    {
        const int *counts = lightGrid.tileCountsDataPtr();
        const int *offsets = lightGrid.tileDataPtr();

        for (int i = 0; i < LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y; ++i)
        {
            g_gridBuffer3[i].x = counts[i];
            g_gridBuffer3[i].y = offsets[i];
            //LogMessage ("POTT", "bind_grid(%d), count:%d, offset:%d", i, counts[i], offsets[i]);
            //LogMessage ("POTT", "bind_grid(%d), count:%d, offset:%d", i, g_gridBuffer3[i].x, g_gridBuffer3[i].y);
        }
    }

    glActiveTexture(GL_TEXTURE0 + TDTU_LightGridCountOffsetData);
    // This should not be neccessary, but for amd it seems to be (HD3200 integrated)
    //glBindTexture(GL_TEXTURE_BUFFER, g_lightGridCountOffsetsTexture);
    glBindTexture(GL_TEXTURE_2D, g_lightGridCountOffsetsTexture);
    /*
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG16UI,          //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_UNSIGNED_SHORT,
        //GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
     */

    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG32I,           //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_INT,
        //GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data


    const Lights &lights = lightGrid.getViewSpaceLights();

    const size_t maxLights = NUM_POSSIBLE_LIGHTS;
    //static glm::vec4 light_position_range[maxLights];
    //static glm::vec4 light_color[maxLights];
    //memset(light_position_range, 0, sizeof(light_position_range));

    for (size_t i = 0; i < std::min(maxLights, lights.size()); ++i)
    {
        g_lightPositionRangeBuffer3[i] = glm::vec4(lights[i].position, lights[i].range);
        g_lightColorBuffer3[i] = lights[i].color;
        //LogMessage ("POTT", "color(%d): %f %f %f", i, g_lightColorBuffer3[i].r,g_lightColorBuffer3[i].g,g_lightColorBuffer3[i].b);
        //LogMessage ("POTT", "position / range(%d): %f %f %f %f", i, lights[i].position[0],
        //		lights[i].position[1], lights[i].position[2],lights[i].range);
    }



    for (int i = 0; i < DRTI_Max; ++i)
    {
        bindTexture(GL_TEXTURE_2D, i + TDTU_Diffuse, g_renderTargetTextures[i]);
    }
    CHECK_GL_ERROR();

#if 0
    //g_gridBuffer2.copyFromHost(tmp2, LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y);

    glActiveTexture(GL_TEXTURE0 + TDTU_LightGridCountOffsetData);

    // This should not be neccessary, but for amd it seems to be (HD3200 integrated)
    //glBindTexture(GL_TEXTURE_BUFFER, g_lightGridCountOffsetsTexture);
    glBindTexture(GL_TEXTURE_2D, g_lightGridCountOffsetsTexture);
/*
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG16I,           //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
*/


    //g_gridBuffer2.copyFromHost(tmp2, LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y);

    glActiveTexture(GL_TEXTURE0 + TDTU_LightGridCountOffsetData);

    // This should not be neccessary, but for amd it seems to be (HD3200 integrated)
    //glBindTexture(GL_TEXTURE_BUFFER, g_lightGridCountOffsetsTexture);
    glBindTexture(GL_TEXTURE_2D, g_lightGridCountOffsetsTexture);
/*
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RG16I,           //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RG_INTEGER,  //Interpret the texture data as luminance data
        GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
*/
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGBA32F,         //Store the texture as rgb
        LIGHT_GRID_MAX_DIM_X,               //The texture is 4 pixels wide
        LIGHT_GRID_MAX_DIM_Y,               //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGBA,    //Interpret the texture data as luminance data
        GL_FLOAT,
        //GL_SHORT,//Read the data as an array of unsigned bytes
        g_gridBuffer3); //Use the texturePixels array as data
#endif


    glActiveTexture(GL_TEXTURE0 + TDTU_LightPositionRanges3);
    glBindTexture(GL_TEXTURE_2D, g_lightPositionRangeTexture3);
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGBA32F,         //Store the texture as rgb
        32,             //The texture is 4 pixels wide
        32,             //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGBA,    //Interpret the texture data as luminance data
        GL_FLOAT,//Read the data as an array of unsigned bytes
        g_lightPositionRangeBuffer3);   //Use the texturePixels array as data

    CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE0 + TDTU_LightColors3);
    glBindTexture(GL_TEXTURE_2D, g_lightColorTexture3);
    //gltexbuffer(gl_texture_buffer, gl_rg16i, g_gridbuffer2);
    glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
        0,              //Use the main image (rather than mipmaps)
        GL_RGB16F,          //Store the texture as rgb
        32,             //The texture is 4 pixels wide
        32,             //The texture is 4 pixels high
        0,              //There is no border to the texture
        GL_RGB, //Interpret the texture data as luminance data
        GL_FLOAT,//Read the data as an array of unsigned bytes
        g_lightColorBuffer3);   //Use the texturePixels array as data
    CHECK_GL_ERROR();

    if (lightGrid.getTotalTileLightIndexListLength())
    {
        glActiveTexture(GL_TEXTURE0 + TDTU_LightIndexData);
        memcpy(g_tileLightIndexListsBuffer, lightGrid.tileLightIndexListsPtr(), lightGrid.getTotalTileLightIndexListLength()*sizeof(int));

        glBindTexture(GL_TEXTURE_2D, g_tileLightIndexListsTexture);
        glTexImage2D(GL_TEXTURE_2D, //Create a 2D texture
            0,              //Use the main image (rather than mipmaps)
            GL_R32I,            //Store the texture as rgb
            256,                //The texture is 4 pixels wide
            256,                //The texture is 4 pixels high
            0,              //There is no border to the texture
            GL_RED_INTEGER, //Interpret the texture data as luminance data
            GL_INT,//Read the data as an array of unsigned bytes
            g_tileLightIndexListsBuffer);   //Use the texturePixels array as data
        CHECK_GL_ERROR();
    }
}


void tiled_shading_app::downSampleDepthBuffer(std::vector<glm::vec2> &depthRanges)
{
    //LogMessage ("POTT", "downsample1");
    glBindFramebuffer(GL_FRAMEBUFFER, g_minMaxDepthFbo);
    //LogMessage ("POTT", "downsample2");
    glm::uvec2 tileSize = glm::uvec2(LIGHT_GRID_TILE_DIM_X, LIGHT_GRID_TILE_DIM_Y);
    glm::uvec2 resolution = glm::uvec2(mWidth, mHeight);
    glm::uvec2 gridRes = (resolution + tileSize - glm::uvec2(1,1)) / tileSize;
    //LogMessage ("POTT", "downsample3");

    glViewport(0, 0, gridRes.x, gridRes.y);
    //glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    //const unsigned int init_ui[4] = {0, 1, 0, 1};
    const unsigned int init_ui[2] = {0, 1};
    //glClearBufferuiv(GL_COLOR, 0, init_ui);//.0f, 1.0f, 0.0f, 1.0f);
    glClearBufferuiv(GL_COLOR, 0, init_ui);//.0f, 1.0f, 0.0f, 1.0f);
    //const float init_f[2] = {0.0f, 1.0f};
    //glClearBufferfv(GL_COLOR, 0, init_f);//.0f, 1.0f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERROR();
    //LogMessage ("POTT", "downsample4");

#if 0
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    //LogMessage ("POTT", "downsample5");
    g_downSampleMinMaxShader->begin();
    if (g_numMsaaSamples == 0)
    {
        g_downSampleMinMaxShader->setTexture2D("depthTex", g_renderTargetTextures[DRTI_Depth], 0);
    }
    //LogMessage ("POTT", "downsample6");


    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    //LogMessage ("POTT", "downsample7");
    CHECK_GL_ERROR();

    g_downSampleMinMaxShader->end();
    //LogMessage ("POTT", "downsample8");

    CHECK_GL_ERROR();
    glDepthMask(GL_TRUE);

    glEnable(GL_DEPTH_TEST);
#if 0
    glPopAttrib();
#endif

    //LogMessage ("POTT", "downsample9");
    CHECK_GL_ERROR();



	union fi
	{
		float f[4];
		unsigned int i[4];
	};

	std::vector<union fi> depthRanges2;
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	int size = gridRes.x * gridRes.y;
	depthRanges2.resize(size);
	depthRanges.resize(size);
	glReadPixels(0, 0, gridRes.x, gridRes.y, GL_RGBA_INTEGER, GL_UNSIGNED_INT, &depthRanges2[0].i[0]);
	CHECK_GL_ERROR();

	for (int i = 0; i<size; i++)
	{
		depthRanges[i].x = depthRanges2[i].f[0];
		depthRanges[i].y = depthRanges2[i].f[1];

	}

	/*
    std::vector<glm::vec2> depthRanges2;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    int size = gridRes.x * gridRes.y;
    depthRanges2.resize(size);
    depthRanges.resize(size);
    glReadPixels(0, 0, gridRes.x, gridRes.y, GL_RG, GL_FLOAT, &depthRanges2[0]);
    for (int i = 0; i<size; i++)
    {
        depthRanges[i] = glm::vec2(depthRanges2[i].x, depthRanges2[i].y);
        //LogMessage ("POTT", "depth range [%d] (%f %f)", i,  depthRanges2[i].x, depthRanges2[i].y );
    }
    //glReadPixels(0, 0, gridRes.x, gridRes.y, GL_RG, GL_FLOAT, &depthRanges[0]);
    CHECK_GL_ERROR();
    */

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
    if (mNumMSAASamples>1)
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
    else
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

}




#if NEW_OBJ_LOADER
void tiled_shading_app::init_buffers()
{

    CHECK_GL_ERROR();
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    // Index buffer object
    glGenBuffers(1, &g_indicesIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (GLuint)* g_model2->getNumberOfIndices(),
        g_model2->getIndexBuffer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    // Position VBO
    glGenBuffers(1, &g_positionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_positionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat)* 3 * g_model2->getNumberOfVertices(),
        g_model2->getVertexBufferPosition(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof (GLfloat), (const void *)NULL);
    glEnableVertexAttribArray(0);


    // Normal VBO
    glGenBuffers(1, &g_normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat)* 3 * g_model2->getNumberOfVertices(),
        g_model2->getVertexBufferNormal(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT,
        GL_TRUE, 3 * sizeof (GLfloat), (const void *)NULL);
    glEnableVertexAttribArray(1);



    // TexCoord VBO
    glGenBuffers(1, &g_texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat)* 2 * g_model2->getNumberOfVertices(),
        g_model2->getVertexBufferTexCoord(), GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT,
        GL_FALSE, 2 * sizeof (GLfloat), (const void *)NULL);
    glEnableVertexAttribArray(2);

    //Tangent VBO
    glGenBuffers(1, &g_tangentsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_tangentsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat)* 3 * g_model2->getNumberOfVertices(),
        g_model2->getVertexBufferTangent(), GL_STATIC_DRAW);

    glVertexAttribPointer(3, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof (GLfloat), (const void *)NULL);
    glEnableVertexAttribArray(3);


    //Bitangent VBO
    glGenBuffers(1, &g_bitangentsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_bitangentsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat)* 3 * g_model2->getNumberOfVertices(),
        g_model2->getVertexBufferBitangent(), GL_STATIC_DRAW);

    glVertexAttribPointer(4, 3, GL_FLOAT,
        GL_FALSE, 3 * sizeof (GLfloat), (const void *)NULL);
    glEnableVertexAttribArray(4);

    // Reset to the default VAO
    glBindVertexArray(0);


    // store material uniforms in buffer object.
    std::vector<MaterialProperties_Std140> tmpMaterials;
    //GLint uniformBufferAlignment;


    CHECK_GL_ERROR();
    for (int i = 0; i < g_model2->getNumberOfMaterials(); i++) {
        ModelOBJ::Material *material = g_model2->getMaterial(i);

        for (int texNum = 0; texNum < 4; texNum++)
        {
            TextureMaps *currentMap;
            std::string texFile = "";

            switch (texNum)
            {
            case 0:
                currentMap = m_diffuseMaps;
                texFile = material->colorMapFilename;
                break;
            case 1:
                currentMap = m_opacityMaps;
                texFile = material->opacityMapFilename;
                material->opaque = true; //default setting (opaque)
                break;
            case 2:
                currentMap = m_specularMaps;
                texFile = material->specularMapFilename;
                break;
            case 3:
                currentMap = m_normalMaps;
                texFile = material->bumpMapFilename;
                break;

            }

            if (texFile.empty() ||
                currentMap->findTextureMap(g_sceneDirectory + texFile))
                continue;

            // load ktx textures
            GLuint texId = 0;
            GLenum target;
            GLboolean isMipmapped;
            GLenum glerror;
            KTX_error_code ktxerror;
            ktxerror = ktxLoadTextureN((g_sceneDirectory + texFile).c_str(), &texId, &target, NULL, &isMipmapped, &glerror,
                0, NULL);
            if (KTX_SUCCESS == ktxerror) {
                currentMap->setMapConfigs(g_sceneDirectory + texFile, texId, target, isMipmapped);
                if (texNum == 1) material->opaque = false; // transparent/translucent object
            }
        }

        MaterialProperties_Std140 tmp;
        tmp.diffuse_color = glm::vec3(material->diffuse[0], material->diffuse[1], material->diffuse[2]);
        tmp.specular_color = glm::vec3(material->specular[0], material->specular[1], material->specular[2]);
        tmp.emissive_color = glm::vec3(0, 0, 0); //temp
        tmp.specular_exponent = material->shininess;

        material->materialIndex = i;
        //m.offset = tmpMaterials.size();

        tmpMaterials.push_back(tmp);
    }
    m_materialPropertiesBuffer.init(tmpMaterials.size(), &tmpMaterials[0], GL_STATIC_DRAW);

}

void tiled_shading_app::render_model(bool alphaTested) //uint32_t renderFlags)
{

    //glPushAttrib(GL_ALL_ATTRIB_BITS);

    glBindVertexArray(g_vao);


        int curBindedTex = 0;
        glActiveTexture(GL_TEXTURE1);
        for (int i = 0; i < g_model2->getNumberOfMeshes(); i++) {

            ModelOBJ::Mesh mesh = g_model2->getMesh(i);

            uint32_t currentRenderFlag = mesh.pMaterial->opaque ? RF_Opaque2 : RF_AlphaTested2;

            if (alphaTested != mesh.pMaterial->opaque) //renderFlags & currentRenderFlag)
            {
                if (!mesh.pMaterial->colorMapFilename.empty()) {
                    TextureMaps::MapConfigs *map_configs = m_diffuseMaps->getMapConfigs(g_model2->getPath() + mesh.pMaterial->colorMapFilename);
                    if (curBindedTex != map_configs->id) {
                        glBindTexture(GL_TEXTURE_2D, map_configs->id);
                        if (map_configs->isMipmapped)
                            /* Enable bilinear mipmapping */
                            glTexParameteri(map_configs->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                        else
                            glTexParameteri(map_configs->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(map_configs->target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                        curBindedTex = map_configs->id;
                    }
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    curBindedTex = 0;
                }

                m_materialPropertiesBuffer.bindSlotRange(GL_UNIFORM_BUFFER, 0, (uint32_t)mesh.pMaterial->materialIndex);
                CHECK_GL_ERROR();
                glDrawElements(GL_TRIANGLES, mesh.triangleCount * 3, GL_UNSIGNED_INT, (unsigned int *)NULL + mesh.startIndex);
                CHECK_GL_ERROR();

                printf("%d \n", i);
            }
        }


    glBindVertexArray(0);

}
#endif








#endif


void tiled_shading_app::updateFPS(float fFPS){
    //FPS 설정
    // JavaVM을 보존
    javaVM = app_->activity->vm;
    if( javaVM->AttachCurrentThread(&jni, NULL) != JNI_OK){
        return;
    }
    javaVM->GetEnv(reinterpret_cast<void**>(&jni), JNI_VERSION_1_6);

    jclass clazz = jni->GetObjectClass(app_->activity->clazz);
    jmethodID methodID = jni->GetMethodID(clazz, "updateFPS", "(F)V");
    jni->CallVoidMethod(app_->activity->clazz, methodID, fFPS);

    // JavaVM에서 스레드 분리
    javaVM->DetachCurrentThread();
}









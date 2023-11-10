/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gles3jni.h"
#include <EGL/egl.h>
#include <Common/Include/Renderer.h>
#include <Common/Include/common.h>

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3


class Renderer_Tiled_Shading: public Renderer {
public:
    Renderer_Tiled_Shading();
    virtual ~Renderer_Tiled_Shading();
    bool init();


private:
    virtual float* mapOffsetBuf();
    virtual void unmapOffsetBuf();
    virtual float* mapTransformBuf();
    virtual void unmapTransformBuf();
    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;

    RendererForLBench *rendererForLBench;
};

Renderer* createTiledShadingRenderer() {
    Renderer_Tiled_Shading* renderer = new Renderer_Tiled_Shading;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

Renderer_Tiled_Shading::Renderer_Tiled_Shading()
:   mEglContext(eglGetCurrentContext()),
    mProgram(0)
{
}

bool Renderer_Tiled_Shading::init() {
    //mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    //if (!mProgram)
    //    return false;
    rendererForLBench = new RendererForLBench();
    LogMessage("Light2", "%d", myPossibleLights);
    rendererForLBench->setLights(myPossibleLights);
    rendererForLBench->changeViewD(xPos, yPos);
    rendererForLBench->setBench(RendererForLBench::BENCH_TILED_SHADING);

    rendererForLBench->start();

 //   rendererForLBench->setLights(myPossibleLights);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

Renderer_Tiled_Shading::~Renderer_Tiled_Shading() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */

    if (rendererForLBench) delete rendererForLBench;
    rendererForLBench = 0;
    if (eglGetCurrentContext() != mEglContext)
        return;
}


float* Renderer_Tiled_Shading::mapOffsetBuf() {
    return NULL;
 ;
}

void Renderer_Tiled_Shading::unmapOffsetBuf() {
}

float* Renderer_Tiled_Shading::mapTransformBuf() {
    return NULL;
}

void Renderer_Tiled_Shading::unmapTransformBuf() {
}

void Renderer_Tiled_Shading::draw(unsigned int numInstances)
{
    rendererForLBench->setLights(myPossibleLights);
    rendererForLBench->changeViewD(xPos, yPos);
    rendererForLBench->update();
    return;
}





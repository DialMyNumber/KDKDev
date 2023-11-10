/*
 * Bench.h
 *
 *  Created on: 2014. 11. 14.
 *      Author: ys.suh
 */

#ifndef COMMON_BENCH_H_
#define COMMON_BENCH_H_

#include <Common/Include/common.h>
#include <string>

class Bench {
public:
    Bench(int width, int height);
    virtual ~Bench();
    virtual void Draw() = 0;
    virtual bool Update() = 0;
    virtual void Shutdown() = 0;
    virtual void SetLights(int nums) = 0;
    virtual void changeViews(float x, float y) = 0;
    void DrawFrame();
    void Terminate();
    int mNumMSAASamples;

protected:
    GLuint LoadShader ( int type, const char *shaderSrc );
    void ResetGLStates();
    int mWidth;
    int mHeight;
    bool mInitialized;
    std::string mResPath;
    GLuint mDefaultFbo;
    GLuint mDefaultFboMSAA;


private:
    void InitFbo();
    void DrawToScreen();
    GLuint mDefaultRb;
    GLuint mDefaultTex;
    GLuint mDefaultRbMSAA;
    GLuint mDefaultTexMSAA;
#if !USE_BLIT
    GLuint mQuadVao;
    GLuint mQuadBuffer;
    GLuint mBlitShader;
    GLuint mBlitShaderSrcTexLoc;
#endif
    int mScreenWidth;
    int mScreenHeight;

};

#endif /* COMMON_BENCH_H_ */

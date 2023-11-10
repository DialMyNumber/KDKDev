/*
 * Bench.cpp
 *
 *  Created on: 2014. 11. 14.
 *      Author: ys.suh
 */

#include "Common/Include/Bench.h"
#include <stdlib.h>

Bench::Bench(int width, int height) {
    mScreenWidth = width;
    mScreenHeight = height;
    mWidth = 2400;
    mHeight =1080;
    mInitialized = false;
    mNumMSAASamples = 4; //it should be maximum number of MSAA samples for InitFBO()
#ifdef ANDROID
    mResPath = std::string(android_fopen_get_res_path()) + "/";
#else
    mResPath = "";
#endif
    InitFbo();
}

Bench::~Bench() {
    glDeleteTextures(1, &mDefaultTex);
    glDeleteRenderbuffers(1, &mDefaultRb);
    glDeleteFramebuffers(1, &mDefaultFbo);
    glDeleteTextures(1, &mDefaultTexMSAA);
    glDeleteRenderbuffers(1, &mDefaultRbMSAA);
    glDeleteFramebuffers(1, &mDefaultFboMSAA);
#if !USE_BLIT
    glDeleteVertexArrays(1, &mQuadVao);
    glDeleteBuffers(1, &mQuadBuffer);
    glDeleteProgram(mBlitShader);
#endif
}

void Bench::ResetGLStates() {

    glClearColor(0.0,0.0,0.0,1.0);
    glClearDepthf(1.0f);

    if (mNumMSAASamples>1)
    {
    	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_TRUE);
    glDisable(GL_DITHER);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    //intialize texture binding
    for (int i = 0; i < 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    for (int i = 0; i < 16; i++) {
        glDisableVertexAttribArray(i);
    }

#if USE_VAO
    glBindVertexArray(0);
#endif
}

GLuint Bench::LoadShader ( int type, const char *shaderSrc )
{
    return esLoadShader(type, shaderSrc);
}

void Bench::DrawFrame() {
    //LogMessage("POTT", "number of MSAA samples ; %d", mNumMSAASamples);

	if (mNumMSAASamples>1)
		glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);

	else
		glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

    Draw();
    DrawToScreen();
}

void Bench::Terminate() {
    Shutdown();
    ResetGLStates();
}

void Bench::InitFbo() {
	glGenFramebuffers(1, &mDefaultFboMSAA);
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFboMSAA);

    glGenRenderbuffers(1, &mDefaultTexMSAA);
    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultTexMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mNumMSAASamples, GL_RGBA8, mWidth, mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mDefaultTexMSAA);


    glGenRenderbuffers(1, &mDefaultRbMSAA);
    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultRbMSAA);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mNumMSAASamples, GL_DEPTH_COMPONENT16, mWidth, mHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDefaultRbMSAA);

    static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, draw_buffers);

    glClearColor(0.0,0.0,0.0,1.0);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	glGenFramebuffers(1, &mDefaultFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFbo);

#if 0
    glGenTextures(1, &mDefaultTex);
    glBindTexture(GL_TEXTURE_2D, mDefaultTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mDefaultTex, 0);
#endif
	glGenRenderbuffers(1, &mDefaultTex);
	glBindRenderbuffer(GL_RENDERBUFFER, mDefaultTex);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mDefaultTex);

	glDrawBuffers(1, draw_buffers);

	glGenRenderbuffers(1, &mDefaultRb);
	glBindRenderbuffer(GL_RENDERBUFFER, mDefaultRb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDefaultRb);

	glClearColor(0.0,0.0,0.0,1.0);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


#if !USE_BLIT
    mBlitShader = esLoadProgramFromFile(
            std::string(mResPath + "shaders/passthru.vs").c_str(),
            std::string(mResPath + "shaders/passthru.fs").c_str());
    mBlitShaderSrcTexLoc = glGetUniformLocation(mBlitShader, "srcTex");

    const float vQuad[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &mQuadVao);
    glBindVertexArray(mQuadVao);

    glGenBuffers(1, &mQuadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vQuad), vQuad, GL_STATIC_DRAW);
    glVertexAttribPointer(BENCH_VERTEX_POSITION, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void *)0);
    glEnableVertexAttribArray(BENCH_VERTEX_POSITION);
    glVertexAttribPointer(BENCH_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GL_FLOAT), (const void *)(sizeof (GL_FLOAT) * 2));
    glEnableVertexAttribArray(BENCH_TEXCOORD);
    glBindVertexArray(0);
#endif

    mNumMSAASamples = 1; //default is no MSAA
}


void Bench::DrawToScreen() {
#if USE_BLIT

	if (mNumMSAASamples>1)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultFboMSAA);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFbo);
		glBlitFramebuffer(0, 0, mWidth, mHeight,
				0, 0, mWidth, mHeight,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, mDefaultFbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, mWidth, mHeight,
			0, 0, mScreenWidth, mScreenHeight,
			GL_COLOR_BUFFER_BIT, GL_LINEAR);
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, mScreenWidth, mScreenHeight);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(mBlitShader);
    glUniform1i(mBlitShaderSrcTexLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mDefaultTex);

    glBindVertexArray(mQuadVao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
#endif
}


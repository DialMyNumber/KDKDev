#include <thread>
#include <mutex>
#include <android/native_window.h>
#include <Common/Include/common.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <Common/Include/Bench.h>
#include <Common/Include/Renderer.h>
#include <Common/Include/Timer.h>

#define MY_MAX_HEIGHT 1080
#define MY_MAX_WIDTH 2400

// benchmarks
#include "tiled_shading.h"
#include "../Include/Renderer.h"
//#include <jniapi.h>

RendererForLBench::RendererForLBench()
    : mMsg(MSG_NONE), mDisplay(0), mSurface(0), mContext(0)
{
    mBench = nullptr;
    mNextBench = nullptr;
    mBenchType = BENCH_NONE;
    mWindow = nullptr;
    mWidth = MY_MAX_WIDTH;
    mHeight = MY_MAX_HEIGHT;
    mRunning = false;
}

RendererForLBench::~RendererForLBench()
{
}

void RendererForLBench::threadStartCallback(void *myself)
{
    RendererForLBench *renderer = (RendererForLBench*)myself;

    renderer->renderLoop();
}

void RendererForLBench::start()
{
    if (mRunning)
        return;

    mThread = std::thread(threadStartCallback, this);
    mRunning = true;
}

void RendererForLBench::stop()
{
    if (!mRunning)
        return;
    // send message to render thread to stop rendering
    mMtx.lock();
    mMsg = MSG_RENDER_LOOP_EXIT;
    mMtx.unlock();

    mThread.join();

    mRunning = false;
}

void RendererForLBench::setWindow(ANativeWindow *window)
{
    mMtx.lock();
    mMsg = MSG_WINDOW_SET;
    mWindow = window;
    mMtx.unlock();
}

void RendererForLBench::setNextBench(void *myself)
{
    RendererForLBench *renderer = (RendererForLBench*)myself;

    BENCH_TYPE nextBench = renderer->getNextBench();
    LogMessage ("POTT", "nextBench: %d", nextBench);
    renderer->setBench(nextBench);
}

RendererForLBench::BENCH_TYPE RendererForLBench::getNextBench() {
#if USE_TIMER
    mMtx.lock();
#endif
    BENCH_TYPE nextBench = (BENCH_TYPE)(mBenchType + 1);
#if USE_TIMER
    mMtx.unlock();
#endif
    return nextBench;
}

void RendererForLBench::setBench(BENCH_TYPE type)
{
#if USE_TIMER
    mMtx.lock();
#endif

    mBenchType = type;
#if USE_TIMER
    mMsg = MSG_CHANGE_BENCH;
    mMtx.unlock();
#else
    //changeBench();
    mBench = std::auto_ptr<tiled_shading_app>(new tiled_shading_app(mWidth, mHeight, myPossibleLights, xPos, yPos));
    LogMessage ("POTT", "mBench is Tiled shading");

#endif
}

void RendererForLBench::changeBench()
{
    bool needsRenderResult = false;
    if (mBench) {
        mBench->Terminate();
        mBenchEndTime = std::chrono::system_clock::now();
        //eglSwapBuffers(mDisplay, mSurface);
        needsRenderResult = true;
    }

    if (mBenchType < BENCH_SIZE) {
        switch (mBenchType) {

        case BENCH_TILED_SHADING:
            mTitle = "Tiled Shading";
            mBench = std::auto_ptr<tiled_shading_app>(new tiled_shading_app(mWidth, mHeight, myPossibleLights, xPos, yPos));
            LogMessage("2nd", "mBench");
            break;


        }
    } else {
        mBenchType = BENCH_NONE;
        mBench = nullptr;
        mTitle = "Result";
    }

    if (needsRenderResult) {
#if USE_TIMER
        //setBenchResult((float)mFrames / 10.f, mTitle);
#else
        std::chrono::duration<double> elapsed_seconds = mBenchEndTime - mBenchStartTime;
        //setBenchResult((float)mFrames / elapsed_seconds.count(), mTitle);
#endif
        waitWhileRenderResult();
    }

    if (mBenchType == BENCH_NONE) {
        //setDone();
    }

    mFrames = mPreFrames = 0;
}

void RendererForLBench::waitWhileRenderResult() {
    std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endTime-mBenchEndTime;
    if (2.0 - elapsed_seconds.count() > 0)
        std::this_thread::sleep_for(std::chrono::duration<double>(2.0 - elapsed_seconds.count()));
}

#if USE_TIMER
void Renderer::setTimer()
{
    mTimer = std::auto_ptr<Timer>(new Timer(setNextBench, (void*)this));
    mTimer->setSingleShot(true);
    mTimer->setInterval(Timer::Interval(10000));
    mTimer->start(true);
}
#endif

void RendererForLBench::renderLoop()
{
    bool renderingEnabled = true;

    while (renderingEnabled) {

        mMtx.lock();

        // process incoming messages
        switch (mMsg) {

            case MSG_WINDOW_SET:
                initialize();
                changeBench();
                break;

            case MSG_RENDER_LOOP_EXIT:
                renderingEnabled = false;
                destroy();
                break;

            case MSG_CHANGE_BENCH:
                changeBench();
                break;

            default:
                break;
        }
        mMsg = MSG_NONE;

        if (mDisplay && mBenchType != BENCH_NONE) {

            if (mBench->Update()) {
                //setBenchTitle(mTitle);
#if USE_TIMER
                setTimer();
#else
                mBenchStartTime = std::chrono::system_clock::now();
#endif
            }

            mBench->DrawFrame();
            eglSwapBuffers(mDisplay, mSurface);
            mFrames ++;
#if !USE_TIMER
            if (mFrames == 50) {
                BENCH_TYPE nextBench = getNextBench();
                //setBench(nextBench);
            }
#endif
        }

        mMtx.unlock();
    }
}

#define EGL_CONTEXT_MINOR_VERSION_KHR           0x30FB
bool RendererForLBench::initialize()
{
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_RED_SIZE, 5,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3,
                                EGL_CONTEXT_MINOR_VERSION_KHR, 1,
                                EGL_NONE };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;


    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        return false;
    }
    if (!eglInitialize(display, 0, 0)) {
        return false;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        destroy();
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        destroy();
        return false;
    }

    ANativeWindow_setBuffersGeometry(mWindow, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, mWindow, 0))) {
        destroy();
        return false;
    }

    if (!(context = eglCreateContext(display, config, 0, contextAttribs))) {
        destroy();
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        destroy();
        return false;
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &mWidth) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &mHeight)) {
        destroy();
        return false;
    }

    mDisplay = display;
    mSurface = surface;
    mContext = context;

    mBenchType = (BENCH_TYPE)1;
    mFrames = 0;
    mPreFrames = 0;

    return true;
}

void RendererForLBench::destroy() {
    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(mDisplay, mContext);
    eglDestroySurface(mDisplay, mSurface);
    eglTerminate(mDisplay);

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
}

float RendererForLBench::getCurFps(int interval) {
    mMtx.lock();
    int frames = mFrames - mPreFrames;
    mPreFrames = mFrames;
    float fps = 1000.f * frames / (float)interval;
    mMtx.unlock();

    return fps;
}

void RendererForLBench::update()
{
    mBench->SetLights(myPossibleLights);
    mBench->changeViews(xPos, yPos);
    if (mBench->Update()) {
        //setBenchTitle(mTitle);
#if USE_TIMER
        setTimer();
#else
        mBenchStartTime = std::chrono::system_clock::now();
#endif
    }

    mBench->DrawFrame();
    //eglSwapBuffers(mDisplay, mSurface);
    mFrames ++;
}
void RendererForLBench::setLights(int nums){
    myPossibleLights = nums;
    //LogMessage("Light3", "%d", myPossibleLights);
}
void RendererForLBench::changeViewD(float x, float y){
    xPos = x;
    yPos = y;
}

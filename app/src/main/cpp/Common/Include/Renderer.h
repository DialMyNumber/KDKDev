#ifndef RENDERER_H
#define RENDERER_H

#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <Common/Include/Bench.h>
#include <Common/Include/Timer.h>


class RendererForLBench {

public:
    enum BENCH_TYPE {
        BENCH_NONE = 0,

        BENCH_TESSELLATION,
        BENCH_SIZE,
        BENCH_INSTANCING,
        BENCH_SSAO,
        BENCH_TILED_SHADING,
        BENCH_CHC,
        BENCH_CSM,
        BENCH_OIT,


        BENCH_INSTANCING_MSAA,
        BENCH_CHC_MSAA,
        BENCH_CSM_MSAA,






        BENCH_TRIANGLE,
    };

    RendererForLBench();
    virtual ~RendererForLBench();

    void start();
    void stop();
    void setWindow(ANativeWindow* window);
    void setBench(BENCH_TYPE type);
    BENCH_TYPE getNextBench();
    static void setNextBench(void *myself);
    float getCurFps(int interval);
    void update();
    int myPossibleLights;
    void setLights(int nums);
    float xPos;
    float yPos;
    void changeViewD(float x, float y);
private:

    enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT,
        MSG_CHANGE_BENCH
    };

    std::thread mThread;
    std::mutex mMtx;
    enum RenderThreadMessage mMsg;

    // android window, supported by NDK r5 and newer
    ANativeWindow* mWindow;

    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    std::unique_ptr<Bench> mBench;
    std::unique_ptr<Bench> mNextBench;
    BENCH_TYPE mBenchType;
    int mWidth;
    int mHeight;
    std::unique_ptr<Timer> mTimer;
    bool mRunning;
    int mFrames;
    int mPreFrames;
    std::string mTitle;
    std::chrono::time_point<std::chrono::system_clock> mBenchEndTime;
#if !USE_TIMER
    std::chrono::time_point<std::chrono::system_clock> mBenchStartTime;
#endif

    // RenderLoop is called in a rendering thread started in start() method
    // It creates rendering context and renders scene until stop() is called
    void renderLoop();

    bool initialize();
    void destroy();


    // thread functions
    void changeBench();
#if USE_TIMER
    void setTimer();
#endif
    void waitWhileRenderResult();

    // Helper method for starting the thread
    static void threadStartCallback(void *myself);
};

#endif // RENDERER_H


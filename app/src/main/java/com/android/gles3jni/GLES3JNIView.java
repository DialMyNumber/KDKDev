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

package com.android.gles3jni;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import java.util.ArrayList;
import java.util.List;


class GLES3JNIView extends GLSurfaceView implements Subject {
    private static final String TAG = "GLES3JNI";
    private static final boolean DEBUG = true;
    public static int count = 0;
    public static long before;
    public static long now;
    public static float xPos;
    public static float yPos;
    public static float cfps;

    private static List<Observer> observers = new ArrayList<>();

    public GLES3JNIView(Context context) {
        super(context);
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
    }
/*
    public void touchcountplus(){
        count++;
    }
*/
    public void touchcountplus(int i){
        count = i;
    }

    public void touchPos(float x, float y){
        xPos = x;
        yPos = y;
    }

    public static float calcFPS(){
        float fps;
        long x;
        now = System.currentTimeMillis();
        x = now - before;
        fps = 1000 / x;
        GLES3JNIActivity.updateFPS(fps);

        before = now;


        return fps;
    }
    public float printFPS(){
        return cfps;
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            cfps = calcFPS();
            GLES3JNILib.step();
            GLES3JNILib.rebuildingLights(count);
            GLES3JNILib.changeViews(xPos, yPos);
            System.out.println("FPS: " + cfps);
            //mTextView.setText("FPS: " + cfps);
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLES3JNILib.resize(height, width);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            GLES3JNILib.init();
        }
    }

    @Override
    public void registerObserver(Observer observer){
        observers.add(observer);
    }

    @Override
    public void removeObserver(Observer observer){
        observers.remove(observer);
    }

    @Override
    public void notifyObservers(){
        for (Observer observer : observers) {
            observer.update(cfps);
        }
    }

    public void setFPS(){
        this.notifyObservers();
    }


}

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

import static android.content.ContentValues.TAG;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.NativeActivity;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

//added
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import android.content.res.AssetManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import java.time.LocalTime;

import java.util.ArrayList;
import java.util.List;

public class GLES3JNIActivity extends Activity implements Observer{
    // ConcreteObserver (구체적인 관찰자) 클래스

    GLES3JNIView mView;
    public static TextView mTextView = null;
    SeekBar seekBar;
    public static float fps;

    private LinearLayout container;
    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mView = new GLES3JNIView(getApplication());
        setContentView(mView);
        mView.registerObserver(this);

        mTextView = new TextView(this);
        //mTextView.setText("FPS: " + fps);
        Timer timer = new Timer();
        TimerTask timerTask = new TimerTask(){
            @Override public void run(){
                fps = mView.printFPS();
                mTextView.setText("FPS: " + fps);
                //이렇게 하면 될꺼라고 생각했는데
            }
        };
//        timer.schedule(timerTask, 0, 100); //Timer 실행
        mTextView.setTextColor(Color.WHITE);
        mTextView.setTextSize(TypedValue.COMPLEX_UNIT_SP, 32);

        FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
        params.gravity = Gravity.START;
        addContentView(mTextView, params);

        TextView mTextView2 = new TextView(this);
        mTextView2.setTextColor(Color.WHITE);
        mTextView2.setTextSize(TypedValue.COMPLEX_UNIT_SP, 32);
        FrameLayout.LayoutParams params3 = new FrameLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
        );
        params3.gravity = Gravity.TOP | Gravity.END; // 오른쪽 상단으로 배치
        addContentView(mTextView2, params3);

        seekBar = new SeekBar(this);
        seekBar.setMax(1024);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                mView.touchcountplus(seekBar.getProgress());
                mTextView2.setText("Lights: " + seekBar.getProgress());
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                mView.touchcountplus(seekBar.getProgress());
                mTextView2.setText("Lights: " + seekBar.getProgress());
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mView.touchcountplus(seekBar.getProgress());
                mTextView2.setText("Lights: " + seekBar.getProgress());
            }
        });

        FrameLayout.LayoutParams params2 = new FrameLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
        );
        params2.gravity = Gravity.BOTTOM;
        addContentView(seekBar, params2);

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        GLES3JNILib.nativeSetResPath(getExternalFilesDir(null).getAbsolutePath());
        Log.d("POTT", getExternalFilesDir(null).getAbsolutePath());

        try {
            CopyAssets("shaders");
            CopyAssets("scenes");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        Log.d( TAG, "onTouchEvent onView" );

        boolean ret = false;

        // 터치가 되고 있는 위치
        // 얼마나 움직이는지를 계산하려면 이 값을 계속 유지하여 계산해야 됨
        float x = event.getX();
        float y = event.getY();

        switch( event.getActionMasked() )
        {
            // 화면에 손가락이 닿음 // 모든 이벤트의 출발점
            case MotionEvent.ACTION_DOWN:
                //Log.d( TAG, "onTouch Down ACTION_DOWN : (" + x +", " + y + ")" );
                mView.touchPos(x, y);
                //mView.touchcountplus();
                // true
                // 다음에 발생할 후속이벤트가 필요하다.
                // 이부분이 제일중요하며, ACTION_UP이 발생할 때까지
                // 이벤트가 발생하면 이 onTouchEvent가 호출 될 것이다.
                ret = true;
                break;

            // 화면에서 손가락을 땜 // 사실상 이벤트의 끝
            case MotionEvent.ACTION_UP:
                //Log.d( TAG, "onTouch Down ACTION_UP: (" + x +", " + y + ")" );

                // click이 되었으니, OnClickListener를 호출
                //performClick();//빨간줄 떠서 주석처리 했는데 실행됨

                // 다음에 발생할 후속이벤트가 필요하다.(하지만 이벤트의 끝이라 의미는 없다)
                ret = true;
                break;


            // 화면에 손가락이 닿은 채로 움직이고 있음(움직일때마다 호출됨)
            case MotionEvent.ACTION_MOVE:
                //Log.d( TAG, "onTouch Down ACTION_MOVE: (" + x +", " + y + ")" );
                mView.touchPos(x, y);
                // 다음에 발생할 후속이벤트가 필요하다.
                // false를 리턴한다고 해도 다른 ACTION_MOVE나 ACTION_UP이 안오는 것은 아니다.
                // 실제 의미가 없는 리턴값이다.
                ret = true;
                break;
        }

        return ret;
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        System.out.println("onResume");
    }


    //added
    private void CopyAssets(String dir) throws IOException {
        File extDir = getExternalFilesDir(null);
        File dirFile = new File(extDir, dir);
        if (!dirFile.exists())
            dirFile.mkdirs();

        AssetManager assetManager = getAssets();
        String[] assets = null;
        try {
            assets = assetManager.list(dir);
        } catch (IOException e) {
            Log.e("POTT", e.getMessage());
        }

        for(String asset : assets) {
            String fullPath = dir + '/' + asset;
            File file = new File(extDir, fullPath);
            Log.d("POTT", "writing " + file);
            if (file.exists())
                continue;

            String subAssets[] = assetManager.list(fullPath);
            if (subAssets.length > 0) {
                CopyAssets(fullPath);
                continue;
            }

            try {
                InputStream is = assetManager.open(fullPath);
                OutputStream os = new FileOutputStream(file);
                copyFile(is, os);
                is.close();
                os.close();
            } catch (IOException e) {
                Log.w("POTT", "Error writing " + file, e);
            }
        }
    }

    private void copyFile(InputStream is, OutputStream os) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while((read = is.read(buffer)) != -1){
            os.write(buffer, 0, read);
        }
    }

    //FPS
    public static void updateFPS(float fps){

        GLES3JNIActivity.fps = fps;
    }


    @Override
    public void update(float fps){
        this.fps = fps;
        //mTextView.setText("FPS: " + this.fps);
    }



}

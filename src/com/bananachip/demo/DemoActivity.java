/*
 * Copyright (C) 2009 The Android Open Source Project
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
package com.bananachip.demo;



import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningTaskInfo;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.os.Bundle;

import java.io.File;
/////////////=================================
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

import com.bananachip.watcher.WatchDog;
import com.bananachip.uninstall.R;

import android.os.Build;
import android.util.Log;
import butterknife.Bind;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class DemoActivity extends Activity
{
    private static final String TAG = "uninstall";
    
    private int mObserverProcessPid = -1;
    
    @Bind(R.id.editText1) EditText mUrlText;
    
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.layout_hellojni);
        ButterKnife.bind(this);
        
        mUrlText.setText("http://www.163.com");
    }
    
    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        
    }
    
    @OnClick(R.id.button1)
    public void submit(Button button) {
      WatchDog.changeUrl(this, mUrlText.getText().toString());
    }
}

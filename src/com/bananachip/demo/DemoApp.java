package com.bananachip.demo;

import com.bananachip.watcher.WatchDog;
import android.app.Application;
import android.util.Log;

public class DemoApp extends Application{

	@Override
	public void onCreate() {
		Log.i("UninstallDemoApp", "onCreate");
		
		try {
			WatchDog.init2("com.bananachip.uninstall", "libwatchDog.so", WatchDog.getUserSerial(this), "http://www.baidu.com");
		} catch (Throwable e) {
			e.printStackTrace();
		}
       	
		
		super.onCreate();
	}

}

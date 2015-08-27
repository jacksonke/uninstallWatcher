package com.bananachip.watcher;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.content.Context;
import android.util.Log;

public class WatchDog {
	public static final String TAG=WatchDog.class.getSimpleName();
	
	static{
		try {
			System.loadLibrary("util");
		} catch (Throwable  e) {
			Log.e(TAG, "failed to load library");
			e.printStackTrace();
		}
	}
	
	public static native int init2(String packageName, String watchDogName, String userSerial, String url);
	
    public static String getUserSerial(Context context)
    {
    	if (context == null){
    		return null;
    	}
    	
        Object userManager = context.getSystemService("user");
        if (userManager == null)
        {
            Log.e(TAG, "userManager not exsit !!!");
            return null;
        }
        
        try
        {
            Method myUserHandleMethod = android.os.Process.class.getMethod("myUserHandle", (Class<?>[]) null);
            Object myUserHandle = myUserHandleMethod.invoke(android.os.Process.class, (Object[]) null);
            
            Method getSerialNumberForUser = userManager.getClass().getMethod("getSerialNumberForUser", myUserHandle.getClass());
            long userSerial = (Long) getSerialNumberForUser.invoke(userManager, myUserHandle);
            Log.i(TAG, "serial number is " + userSerial);
            return String.valueOf(userSerial);
        }
        catch (NoSuchMethodException e)
        {
            Log.e(TAG, "", e);
        }
        catch (IllegalArgumentException e)
        {
            Log.e(TAG, "", e);
        }
        catch (IllegalAccessException e)
        {
            Log.e(TAG, "", e);
        }
        catch (InvocationTargetException e)
        {
            Log.e(TAG, "", e);
        }
        
        return null;
    }
	

}

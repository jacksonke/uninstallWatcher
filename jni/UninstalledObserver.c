#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kesyPrint.h"
#include <errno.h>
#include <unistd.h>
//#include <fcntl.h>
#include <sys/stat.h>

// check if watchDog is existent
// return ret:
// 0 nonexistent
// 1 existent
// -1 something wrong when query
static int checkSingletonV2(const char* dogName)
{
	FILE* stream;
	
	/* Opening a read-only channel to ls command. */
	char tmpCmd[256] = "ps ";
	strcat(tmpCmd, dogName);
	stream = popen(tmpCmd, "r");
	if (NULL == stream)
	{
		kesyPrintf("Unable to execute the command.");
		return -1;
	}
	else
	{
		char buffer[1024];
		int status;
		int line = 0;
		/* Read each line from command output. */
		while (NULL != fgets(buffer, 1024, stream))
		{
			kesyPrintf("read: %s\n", buffer);
			line++;
		}
		/* Close the channel and get the status. */
		status = pclose(stream);
		kesyPrintf("ps exited with status %d\n", status);
		
		if (line >= 2){
			return 1;
		}
		else{
			return 0;
		}
	}
}

// packageName : the package name who call this jni function
// watchDogName 这个是监控程序的so文件名。这个判断有没有监控进程存在的依据
// userSerial 这个参数在弹出网页需要用到，4.2以上的版本用到，4.2的为NULL
// url 这个参数是卸载程序时，弹出的网页网址
JNIEXPORT int JNICALL Java_com_bananachip_watcher_WatchDog_init2(JNIEnv *env, jobject obj,
	jstring packageName, jstring watchDogName, jstring userSerial, jstring url)
{
	char szUrl[NAME_MAX]="\0";
	char szPackageName[NAME_MAX]="\0";
	char szWatchDogName[NAME_MAX]="\0";
	char szSerial[NAME_MAX]="\0";
	
	kesyPrintf("NAME_MAX=%d\n", NAME_MAX);
	
	// 只是简单较错
	if (NULL == packageName){
		kesyPrintf("packageName null\n");
		return -1;
	}
	
	if (NULL == watchDogName){
		kesyPrintf("watchDogName null\n");
		return -1;
	}
	
	// convert to c-style string
	const jbyte* str;
	str = (*env)->GetStringUTFChars(env, packageName, NULL);
	if (0 != str) {
		strcpy(szPackageName, str);
		(*env)->ReleaseStringUTFChars(env, packageName, str);
	}
	else{
		kesyPrintf("failed to convert to C string[packageName]\n");
		return -1;
	}
	
	str = (*env)->GetStringUTFChars(env, watchDogName, NULL);
	if (0 != str) {
		strcpy(szWatchDogName, str);
		(*env)->ReleaseStringUTFChars(env, watchDogName, str);
	}
	else{
		kesyPrintf("failed to convert to C string[watchDogName]\n");
		return -1;
	}
	
	if (userSerial != NULL){
		str = (*env)->GetStringUTFChars(env, userSerial, NULL);
		if (0 != str) {
			strcpy(szSerial, str);
			(*env)->ReleaseStringUTFChars(env, userSerial, str);
		}
		else{
			kesyPrintf("failed to convert to C string[userSerial]\n");
			return -1;
		}
	}
	
	if (url != NULL){
		str = (*env)->GetStringUTFChars(env, url, NULL);
		if (0 != str) {
			strcpy(szUrl, str);
			(*env)->ReleaseStringUTFChars(env, url, str);
		}
		else{
			kesyPrintf("failed to convert to C string[url]\n");
			return -1;
		}
	}
	
	if (checkSingletonV2(szWatchDogName) == 1){
		kesyPrintf("watch dog is existe\n");
		return 0;
	}

    // fork a child process
    pid_t pid = fork();
    if (pid < 0)
    {
		kesyPrintf("fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
		char szSoPath[256]="\0";
		sprintf(szSoPath, "/data/data/%s/lib/%s", szPackageName, szWatchDogName);
        if (strlen(szSerial) == 0)
        {
			kesyPrintf("111111111111111\n");
			execlp(szSoPath, szWatchDogName, "-p", szPackageName, "-u", szUrl, (char *)NULL);
			
			exit(1);
        }
        else
        {
			kesyPrintf("2222222222222\n");
			execlp(szSoPath, szWatchDogName, "-p", szPackageName, "-s", szSerial, "-u", szUrl, (char *)NULL);
        }
		
		kesyPrintf("exec AM command failed !!! errno=%d\n", errno);

    }
    else
    {
        return pid;
    }
}

#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kesyPrint.h"
#include <errno.h>
#include <unistd.h>
//#include <fcntl.h>
#include <sys/stat.h>

// check if watchDog exist
// return ret:
// 0 nonexistent
// 1 existent
// -1 something wrong when query
// TODO: to improve
static int checkSingletonV2(const char* dogName)
{
	FILE* stream;
	
	/* Opening a read-only channel to ps command. */
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

// convert jstring to local C-style string
// myJString: 	jstring to convert
// szLocal: 		store the converted c-style string
// szFailedMsg: log msg when it's failed to convert
// return value: 0 when failed, 1 when success
static int convertJString2LocalString(JNIEnv *env, jstring myJString, char* szLocal, char* szFailedLog)
{
	const jbyte* str;
	str = (*env)->GetStringUTFChars(env, myJString, NULL);

	if (0 != str) {
		strcpy(szLocal, str);
		(*env)->ReleaseStringUTFChars(env, myJString, str);
		return 1;
	}
	else{
		if (szFailedLog != NULL){
			kesyPrintf("%s", szFailedLog);
		}

		return 0;
	}
}

/*
 * Class:     com_bananachip_watcher_WatchDog
 * Method:    init2
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 * packageName : the package name who call this jni function
 * watchDogName 这个是监控程序的so文件名。这个判断有没有监控进程存在的依据
 * userSerial 这个参数在弹出网页需要用到，4.2以上的版本用到，4.2的为NULL
 * url 这个参数是卸载程序时，弹出的网页网址
 */
 JNIEXPORT jint JNICALL Java_com_bananachip_watcher_WatchDog_init2(JNIEnv *env, jclass obj,
	jstring packageName, jstring watchDogName, jstring userSerial, jstring url)
{
	char szUrl[NAME_MAX]="\0";
	char szPackageName[NAME_MAX]="\0";
	char szWatchDogName[NAME_MAX]="\0";
	char szSerial[NAME_MAX]="\0";
	
	int retConvert  = 0;

	kesyPrintf("NAME_MAX=%d\n", NAME_MAX);
	
	if (NULL == packageName){
		kesyPrintf("packageName is null\n");
		return -1;
	}
	
	if (NULL == watchDogName){
		kesyPrintf("watchDogName is null\n");
		return -1;
	}
	

	if (convertJString2LocalString(env, packageName, szPackageName, "failed to convert to C string[packageName]\n") == 0){
		return -1;
	}
	
	if (convertJString2LocalString(env, watchDogName, szWatchDogName, "failed to convert to C string[watchDogName]\n") == 0){
		return -1;
	}
	
	if (userSerial != NULL){
		if (convertJString2LocalString(env, userSerial, szSerial, "failed to convert to C string[userSerial]\n") == 0){
			return -1;
		}
	}
	
	if (url != NULL){
		if (convertJString2LocalString(env, url, szUrl, "failed to convert to C string[url]\n") == 0){
			return -1;
		}
	}
	
	if (checkSingletonV2(szWatchDogName) == 1){
		kesyPrintf("watch dog is existent\n");
		return 0;
	}

    // fork a child process
    pid_t pid = fork();
    if (pid < 0)
    {
		kesyPrintf("fork failed, errno=%d\n", errno);
    }
    else if (pid == 0)
    {
		char szSoPath[256]="\0";
		sprintf(szSoPath, "/data/data/%s/lib/%s", szPackageName, szWatchDogName);
        if (strlen(szSerial) == 0) {
			execlp(szSoPath, szWatchDogName, "-p", szPackageName, "-u", szUrl, (char *)NULL);
        }
        else {
			execlp(szSoPath, szWatchDogName, "-p", szPackageName, "-s", szSerial, "-u", szUrl, (char *)NULL);
        }
		
		kesyPrintf("exec command failed !!! errno=%d\n", errno);

		exit(1);

    }
    else
    {
        return pid;
    }
}

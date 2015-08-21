#include <jni.h>
#include "kesyPrint.h"
#include <errno.h>
#include <getopt.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     (5 * (EVENT_SIZE + 16))

#define ANDROID_MAX_PATH	1024
#define OPTION_STRING "p:u:s:"

#define INVALID_FD	-1


static char gPackageName[64] = "";
static char gUrl[ANDROID_MAX_PATH] = "http://www.baidu.com";
static char gUserSerial[ANDROID_MAX_PATH]="\0";
static int gnVersion = 2;
static char gAccountPath[ANDROID_MAX_PATH] = "\0";

static char urlFoo[] = "urlFoo";

static char gAppDir[ANDROID_MAX_PATH]="\0";

void *p_buf = NULL;

void printerr()
{
	kesyPrintf("errno=%d, desc=%s\n", errno, strerror(errno));
}

void cleanup()
{
	if (p_buf != NULL){
		free(p_buf);
		p_buf = NULL;
	}
}

void popupBrowser(){
	if (strlen(gUserSerial) == 0)
	{
		// am start -a android.intent.action.VIEW -d $(url)
        execlp("am", "am", "start", "-a", "android.intent.action.VIEW", "-d", gUrl, (char *)NULL);
	}
	else
	{
		// am start --user userSerial -a android.intent.action.VIEW -d $(url)
		execlp("am", "am", "start", "--user", gUserSerial, "-a", "android.intent.action.VIEW", "-d", gUrl, (char *)NULL);
	}
	
	kesyPrintf("exec AM command failed !!! errno=%d\n", errno);
}

void goodbye(void)
{
	kesyPrintf("goodbye\n");
	cleanup();
}


// -p: package name
// -u: url to popup
// -s: serial. if API level >=17，userSerialNumber is needed to popup browser
void parse_opt(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, OPTION_STRING)) != -1) {
		switch (opt) {
		case 'p':
			if (optarg != NULL){
				strcpy(gPackageName, optarg);
			}
			else{
				kesyPrintf("Incorrect param -p\n");
			}
			
			break;
		case 'u':
			if (optarg != NULL){
				strcpy(gUrl, optarg);
			}
			else{
				kesyPrintf("Incorrect param -u\n");
			}
			break;
		case 's':
			if (optarg != NULL){
				strcpy(gUserSerial, optarg);
			}
			else{
				kesyPrintf("Incorrect param -s\n");
			}
			break;
		default:
			break;
		}
		
	}
}


int startWatch()
{
	kesyPrintf("observed by child process\n");
	
	kesyPrintf("EVENT_SIZE=%d", EVENT_SIZE);
	kesyPrintf("EVENT_BUF_LEN=%d", EVENT_BUF_LEN);

	if (p_buf != NULL){
		free(p_buf);
		p_buf = NULL;
	}
	
	p_buf = malloc(EVENT_BUF_LEN);
	if (p_buf == NULL)
	{
		kesyPrintf("malloc failed !!!\n");
		exit(1);
	}

	kesyPrintf("start observe\n");
	
	while(1)
	{
		int fileDescriptor = inotify_init();
		if (fileDescriptor < 0)
		{
			kesyPrintf("inotify_init failed\n");
			exit(1);
		}

		int urlFooExist = 1;
		int maskFlag = IN_DELETE_SELF;

		if (access(gAccountPath, F_OK) == -1){
			kesyPrintf("urlFoo does not exist");
			maskFlag |= IN_CREATE;
			urlFooExist = 0;
		}
		
		int wdAppDir = inotify_add_watch(fileDescriptor, gAppDir, maskFlag);
		if (wdAppDir < 0)
		{
			kesyPrintf("inotify_add_watch failed \n");

			if (ENOENT == errno){
				close(fileDescriptor);
				popupBrowser();
			}
			
			
			printerr();
			exit(1);
		}
		
		int wdAccount = -1;
		if (1 == urlFooExist){
			wdAccount = inotify_add_watch(fileDescriptor, gAccountPath, IN_CLOSE_WRITE | IN_DELETE_SELF);
			if (wdAccount < 0){
				kesyPrintf("failed to add watch [urlFoo]\n");
				printerr();
				exit(1);
			}
		}
		else{
			// check if observing folder is existent.
			int n = 0;
			while (n < 10){
				kesyPrintf("[%d]test if app is uninstalled====", n);
				if (-1 == access(gAppDir, F_OK)){
					kesyPrintf("dir does not exist");
					close(fileDescriptor);
					popupBrowser();
					
					exit(1);
				}
				else{
					kesyPrintf("[%d]test if app is uninstalled-------------exist", n);
				}

				++n;

				usleep(10);
			}
			
		}
		
		kesyPrintf("begin read loop\n");
		kesyPrintf("wdAppDir=%d", wdAppDir);
		kesyPrintf("wdAccount=%d", wdAccount);
		memset(p_buf, 0, EVENT_BUF_LEN);

		size_t readBytes = read(fileDescriptor, p_buf, EVENT_BUF_LEN);
		
		if (readBytes <= 0){
			kesyPrintf("failed to read events \n");
			continue;
		}
		
		kesyPrintf("readBytes=%d\n", readBytes);
		
		int k = 0;
		int bExit = 0;
		while (k < readBytes){
			struct inotify_event *event = ( struct inotify_event * )(p_buf + k);
			if (event->len > 0){
				kesyPrintf("event.name=%s\n", event->name);
			}

			// print mask
			kesyPrintf("mask=0x%x, wd=%d\n", event->mask, event->wd);
			
			if (event->wd == wdAccount){
				if (IN_CLOSE_WRITE & event->mask > 0){
					kesyPrintf("AccountInfo change detected!!!");
					
					// read account info
					FILE *f = fopen(gAccountPath, "r");
					if (f != NULL){
						char buf[ANDROID_MAX_PATH];
						if (NULL != fgets(buf, ANDROID_MAX_PATH, f)){
							kesyPrintf("AccountInfo changed:");
							kesyPrintf("%s", buf);
							strcpy(gUrl, buf);
						}
						else{
							kesyPrintf("failed to read url\n");
							printerr();
						}
						fclose(f);
					}
					else{
						kesyPrintf("failed to open file accountInfo\n");
					}
				}
				
				if (IN_DELETE_SELF & event->mask > 0){
					kesyPrintf("urlFoo is deleted");
				}
			}
			
			if (event->wd == wdAppDir){
				if (event->mask & IN_CREATE > 0){
					kesyPrintf("file created:%s", event->name);
				}

				if (IN_DELETE_SELF & event->mask > 0)
				{
					kesyPrintf("app is deleted\n");
					bExit = 1;
					break;
				}

			}
			
			if (IN_IGNORED == event->mask){
				if (event->wd == wdAppDir){
					kesyPrintf("IN_IGNORED dir");
				}
				else{
					kesyPrintf("IN_IGNORED, but not dir fd");
				}
			}
			
			kesyPrintf("k=%d, event->len=%d", k, event->len);
			k += EVENT_SIZE + event->len;
		}
		
		inotify_rm_watch(fileDescriptor, wdAppDir);
		inotify_rm_watch(fileDescriptor, wdAccount);
        close(fileDescriptor);
		
		if (1 == bExit){
			break;
		}
	}

	cleanup();
	
	popupBrowser();
}


void initPath()
{
	strcpy(gAppDir, "/data/data/");
	strcat(gAppDir, gPackageName);
	
	sprintf(gAccountPath, "/data/data/%s/%s", gPackageName, urlFoo);
	
//	strcpy(gAppFilesDir, gAppDir);
//	strcat(gAppFilesDir, "/files");
//	
//	strcpy(gAppObservedFile, gAppDir);
//	strcat(gAppObservedFile, "/observedFile");
//	
//	strcpy(gAppLockFile, gAppDir);
//	strcat(gAppLockFile, "/lockFile");
}

int main(int argc, char**argv)
{
	atexit(goodbye);
	
	parse_opt(argc, argv);
	
	kesyPrintf("argv[0]=%s\n", argv[0]);
	// sprintf(argv[0], "%s_dog", gPackageName);
	strcpy(argv[0], "watchDog");
	kesyPrintf("argv[0]=%s\n", argv[0]);
	kesyPrintf("start watch. Version[%d]\n", gnVersion);
	
	

	kesyPrintf("Parse option:\n");
	kesyPrintf("gPackageName=%s\n", gPackageName);
	kesyPrintf("gUrl=%s\n", gUrl);
	kesyPrintf("gUserSerial=%s\n", gUserSerial);
	
	initPath();
	
	kesyPrintf("initPath:\n");
	kesyPrintf("gAppDir=%s\n", gAppDir);
	kesyPrintf("gAccountPath=%s\n", gAccountPath);
//	kesyPrintf("gAppFilesDir=%s\n", gAppFilesDir);
//	kesyPrintf("gAppObservedFile=%s\n", gAppObservedFile);
//	kesyPrintf("gAppLockFile=%s\n", gAppLockFile);

	startWatch();
}

#ifdef __cplusplus
}
#endif

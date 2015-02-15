// create by kesy 20140526

#include <stdio.h>
#include <stdarg.h>
#include <android/log.h>
#include <time.h>

static const char* TAG = "uninstall";

void kesyPrintf(const char *format, ...)
{
#ifdef KE_DEBUG
    char buf[2048] = "\0";
    
    va_list args;
    va_start(args,format);
    vsprintf(buf + strlen(buf), format, args); 
    va_end(args);

    __android_log_write(ANDROID_LOG_INFO, TAG, buf);
    
#endif
}

#if 0
static FILE* gFile = 0;
void kesyPrintf_v1(const char *format, ...)
{
    if (gFile == 0){
        system("mkdir /mnt/sdcard/111");
        gFile = fopen("/mnt/sdcard/111/kesy.log", "w+");
    }

    if (0 == gFile){
        printf("Failed to open log file\n");
        return;
    }

    char buf[2048] = "\0";

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(buf, "%d-%d-%d %d:%d:%d\t", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    sprintf(buf +strlen(buf), "pid=%d\t", getpid());

    va_list args;
    va_start(args,format);
    vsprintf(buf + strlen(buf), format, args);
    va_end(args);

    printf("%s", buf);
    fprintf(gFile, "%s", buf);
    fflush(gFile);
}
#endif



#ifndef _ALIFE_FILE_H_
#define _ALIFE_FILE_H_
#include <time.h>
#include <psp2/types.h>
#include <psp2/kernel/threadmgr.h>
typedef enum imgType {
    JPG = 0,
    PNG = 1,
    BMP = 2,
    UNKNOWN = 3
} imgType;

enum GAMESCREEN {
    MENU = 0,
    INFO ,
    PLAY ,
    JUMP ,
    SCANNING ,
    OPTION ,
};


int debug_log (char* format, ... );
int getDirList(char *basePath);
int readFileList(char *basePath);
int sortFiles();
int viewFiles();
int nextFile(int cur);
int preFile(int cur);
int getFilePath( int cur);
int testGetfilePath();
int saveCurrentIndex( int cur);
int loadCurrentIndex();
int resetDirInfos();
int saveConfigFile();
int loadConfigFile();
imgType getFileType(char *filename);

#define TIMENOW (time(NULL))

#define MAXSCROLLSPEED 60
#define MINSCROLLSPEED 10
#define SCREEN_WIDTH 960
#define SCREEN_HIGHT 544
#define SCREEN_HALF_WIDTH 480
#define SCREEN_HALF_HIGHT 272
#define MAXLEN 2048
#define MAXGAMES 200
#define MAXFILE 10000
#define SAVEFILE "save.txt"
#define SORTEDFILE "sortindex.txt"
#define FILENAMEFILE "filenames.txt"
#define BACKGROUNDFILE "ux0:comic/bg.png"
#define FONTPGF "ux0:data/font.pgf"
#define CONFIGFILE "ux0:data/ComicView_config.txt"

#ifdef DEBUG
#   define DEBUG_LOG(fmt, args...) debug_log(fmt, ## args)
#else
#   define DEBUG_LOG(fmt, args...) 
#endif

#endif /*  _ALIFE_FILE_H_ */

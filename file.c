#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>


#include "alife_file.h"

extern char baseDir[MAXGAMES][MAXLEN];
extern int totalDirs;
extern int configItem[10];

char img_path[MAXLEN];
char file_names[MAXFILE][MAXLEN];
int file_num=0;
int sort_files[MAXFILE];
int isSorted=0;
char filePath[MAXLEN];
int sorting = 0;
int sorting_index = -1;

int getDirList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    struct stat buf;
    char fullfilepath[MAXLEN];
    if ((dir=opendir(basePath)) == NULL)
    {
        DEBUG_LOG("Open dir[%s] error...\n",basePath);
        return 1;
    }
    memset(baseDir,'\0',sizeof(baseDir));
	totalDirs=0;
    DEBUG_LOG("start getdir\n");
    while ((ptr=readdir(dir)) != NULL)
    {
      DEBUG_LOG("dir:[%s]\n",ptr->d_name);
      strcpy(fullfilepath,basePath);
      if ( fullfilepath[strlen(fullfilepath)-1] != '/') strcat(fullfilepath,"/");
      strcat(fullfilepath,ptr->d_name);
      if (stat(fullfilepath,&buf) != 0 ) DEBUG_LOG("get stat[%s] failed.\n",fullfilepath);
      if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) {    ///current dir OR parrent dir
         //DEBUG_LOG("get stat[%s] is a dir %d .\n",fullfilepath,S_ISDIR(buf.st_mode));
         continue;
      } else if (S_ISDIR(buf.st_mode) ) {
         strcpy(baseDir[totalDirs++],ptr->d_name);
         //DEBUG_LOG("get stat[%s] is a dir.\n",fullfilepath);
      } else {
      }
    }
    closedir(dir);
    return 0;
}

int restoreFromFilenameFile(char *basePath ) {
    char str[MAXLEN];
    char tempname[MAXLEN]; 
    FILE *fp = NULL;
    int res=0;
    memset(file_names,'\0',sizeof(file_names));
	file_num=0;
    DEBUG_LOG("start restore filenames\n");
    strcpy(str,basePath);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,FILENAMEFILE);
    DEBUG_LOG("will read open filenamefile [%s], time %ld\n",str, TIMENOW);
    fp = fopen(str,"r");
    if ( fp == NULL ) return -1;
    DEBUG_LOG("start fscanf\n");
    do {
        res = fscanf(fp, " %[^\n]", tempname);
        //DEBUG_LOG("get filename[%s]\n",tempname);
        if ( res != -1 ) strcpy(file_names[file_num++],tempname);
    } while(!feof(fp));
    fclose(fp);
    DEBUG_LOG("restore filenamefile done, time %ld\n",TIMENOW);
    return 0;
}

int scanFiles(char *basePath) {
    DIR *dir;
    FILE *fp = NULL;
    char str[MAXLEN];
    struct dirent *ptr;
    struct stat buf;
    char fullfilepath[MAXLEN];
    if ((dir=opendir(basePath)) == NULL)
    {
        DEBUG_LOG("Open dir error...\n");
        return 1;
    }
    memset(file_names,'\0',sizeof(file_names));
	file_num=0;
    DEBUG_LOG("start readdir\n");
    strcpy(str,basePath);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,FILENAMEFILE);
    DEBUG_LOG("will write open filenamefile [%s]\n",str);
    fp = fopen(str,"w");
    while ((ptr=readdir(dir)) != NULL)
    {
      DEBUG_LOG("dir:[%s]\n",ptr->d_name);
      strcpy(fullfilepath,basePath);
      if ( fullfilepath[strlen(fullfilepath)-1] != '/') strcat(fullfilepath,"/");
      strcat(fullfilepath,ptr->d_name);
      if (stat(fullfilepath,&buf) != 0 ) DEBUG_LOG("get stat[%s] failed.\n",fullfilepath);
      if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0) {    ///current dir OR parrent dir
         //DEBUG_LOG("get stat[%s] is a dir %d .\n",fullfilepath,S_ISDIR(buf.st_mode));
         continue;
      } else if (S_ISDIR(buf.st_mode) ) {
         //DEBUG_LOG("get stat[%s] is a dir.\n",fullfilepath);
      } else {
        imgType x = getFileType(ptr->d_name);
        if (x != UNKNOWN) {
            strcpy(file_names[file_num++],ptr->d_name);
            if (fp != NULL) fprintf(fp,"%s\n",ptr->d_name);
        }
      }
    }
    if ( fp != NULL ) fclose(fp);
    closedir(dir);
	isSorted=0;
    return 0;

}

int readFileList(char *basePath)
{
    int res = 0;
    res = restoreFromFilenameFile(basePath);
    if (res == 0 ) {
        isSorted = 0;
        return 0;
    }
    return -1;
}
      

int sortFiles(SceSize args_size, void *args) {
	int i,j,tmp,min;
    if (sorting == 1 ) return 0;
    else sorting = 1;
	memset(sort_files,-1,sizeof(sort_files));
    char str[MAXLEN];
    FILE *fp = NULL;
    strcpy(str,img_path);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,SORTEDFILE);
    DEBUG_LOG("will open sortedfile [%s], time %ld\n",str,TIMENOW);
    fp = fopen(str,"w");
    if ( fp == NULL ) DEBUG_LOG("open sortedfile [%s] error.\n",str);
    for (i=0;i<file_num;i++){
		sort_files[i]=i;
	}
	for (i=0;i<file_num-1;i++){
		min=i;
		for(j=i+1;j<file_num;j++) {
			if (strcmp(file_names[sort_files[min]],file_names[sort_files[j]]) > 0 ) {
				min = j;
			}
		}
		if ( min != i ) {
			tmp = sort_files[i];
			sort_files[i]=sort_files[min];
			sort_files[min] = tmp;
		}
        sorting_index = i;
        if ( fp != NULL) fprintf(fp," %d\n", sort_files[i]);
	}
    sorting_index = file_num - 1;
    if ( fp != NULL) fprintf(fp," %d\n", sort_files[file_num-1]);
    if (fp != NULL) fclose(fp);
	isSorted = 1;
    sorting = 0;
    DEBUG_LOG("done sort files. time %ld\n",TIMENOW);
    return sceKernelExitDeleteThread(0);

}

int doSortFiles() {
    // set img_path to dir you want to sort, before calling this function.
    if (sorting == 1 ) return 0;
    SceUID thid = sceKernelCreateThread("sorting_thread", (SceKernelThreadEntry)sortFiles, 0x40, 0x100000, 0, 0, NULL);     
    if (thid >= 0) {
        DEBUG_LOG("start sorting thread, time %ld\n",TIMENOW);
        sceKernelStartThread(thid, 0, NULL);
    }
    DEBUG_LOG("already start sorting thread, will return. time %ld\n",TIMENOW);    
    return 0;
}

int restoreFromSortedFile ( char *basePath ) {
    char str[MAXLEN];
    FILE *fp = NULL;
    strcpy(str,basePath);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,SORTEDFILE);
    DEBUG_LOG("try to read sortedfile [%s]\n",str);
    DEBUG_LOG("restore start time %ld\n",TIMENOW);
    fp = fopen(str,"r");
    int index= -1;
    if (fp != NULL ) {
        memset(sort_files,-1,sizeof(sort_files));
        int i=0;
        int count=0;
        for ( i = 0; i< file_num; i++){
            index = -1;
            fscanf(fp, " %d\n",&index); 
            if ( index != -1 && index < file_num ) {
                sort_files[i] = index; 
                count++;
            }
        }
        fclose(fp);
        DEBUG_LOG("restore end time %ld\n",TIMENOW);
        DEBUG_LOG("restore count %d\n",count);
        if ( count < file_num) isSorted = 0;
        else isSorted = 1;
    }
    return 0;
}

int doinitSortFiles() {
    char str[MAXLEN];
    if ( img_path[strlen(img_path)-1] != '/') strcat(img_path,"/");
    strcpy(str,img_path);
    restoreFromSortedFile(str);
    if ( isSorted == 1 ) return 0;
    sorting_index = -1;
    SceUID thid = sceKernelCreateThread("sorting_thread", (SceKernelThreadEntry)sortFiles, 0x40, 0x100000, 0, 0, NULL);     
    if (thid >= 0)
          sceKernelStartThread(thid, 0, NULL);
    return 0;
}

int resetDirInfos(SceSize args_size, void *args) {
    sorting_index = -1;
    scanFiles(img_path); 
    DEBUG_LOG("re scan files done,will start dosortfiles. time %ld\n",TIMENOW);
    doSortFiles();
    return 0;
}

int viewFiles() {
	int i;
    int c = 30;
	if (!isSorted) doinitSortFiles();
    while ( isSorted == 0  && c-- > 0)
    {
       sceKernelDelayThread(1000*1000);  
    }
	DEBUG_LOG("----------unsorted------------\n");
	for ( i=0;i<file_num;i++){
		printf("%03d:[%s]\n",i,file_names[i]);
	}
	DEBUG_LOG("--------already--sorted-------\n");
	for ( i=0;i<file_num;i++){
		DEBUG_LOG("%03d:[%s]\n",i,file_names[sort_files[i]]);
	}
    return 0;
}

int saveCurrentIndex( int cur) {
    FILE *fp = NULL;
    char str[MAXLEN];
    strcpy(str,img_path);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,SAVEFILE);
    fp = fopen(str,"w");
    if (fp == NULL) return -1;
    fprintf(fp," %d", cur);
    fclose(fp);
    return 0;
}

int loadCurrentIndex() {
    FILE *fp = NULL;
    char str[MAXLEN];
    int res = -1;
    strcpy(str,img_path);
    if ( str[strlen(str)-1] != '/') strcat(str,"/");
    strcat(str,SAVEFILE);
    fp = fopen(str,"r");
    if (fp == NULL) return -1;
    fscanf(fp," %d", &res);
    fclose(fp);
    return res;
}

int loadConfigFile ( ) {
    char str[MAXLEN];
    FILE *fp = NULL;
    strcpy(str,CONFIGFILE);
    fp = fopen(str,"r");
    int index= -101;
    if (fp != NULL ) {
        int i=0;
        for ( i = 0; i< 4; i++){
            fscanf(fp, " %d\n",&index); 
            configItem[i] = index; 
            DEBUG_LOG("load configItem[%d] %d\n",i, configItem[i]);
        }
        fclose(fp);
        return 0;
    }
    return -1;
}

int saveConfigFile ( ) {
    char str[MAXLEN];
    FILE *fp = NULL;
    strcpy(str,CONFIGFILE);
    fp = fopen(str,"w");
    if (fp != NULL ) {
        int i=0;
        for ( i = 0; i< 4; i++){
            DEBUG_LOG("save configItem[%d] %d\n",i, configItem[i]);
            fprintf(fp, " %d\n",configItem[i]); 
        }
        fclose(fp);
        return 0;
    }
    DEBUG_LOG("open [%s] failed\n",CONFIGFILE);
    return -1;
}

int nextFile(int cur) {
	if ( cur < 0 || cur >= file_num-1 ) return -1;
	if (!isSorted) doinitSortFiles();
	return cur+1;
}

int preFile(int cur) {
	if ( cur <= 0 || cur > file_num ) return -1;
	if (!isSorted) doinitSortFiles();
	return cur-1;
}


int getFilePath( int cur) {
	if ( cur < 0 || cur >= file_num ) {
		memset(filePath,'\0',sizeof(filePath));
		return -1;
	}
    if (!isSorted) doinitSortFiles();
	strcpy(filePath,img_path);
	if ( filePath[strlen(filePath)-1] != '/') strcat(filePath,"/");
    if ( isSorted ) {
        strcat(filePath,file_names[sort_files[cur]]);
        return 0;
    } else {
        int c = 30;
        while ( c-- > 0 && sorting_index < cur) {
            sceKernelDelayThread(1000*1000); 
        }
        if ( sorting_index >= cur) {
            strcat(filePath,file_names[sort_files[cur]]);
            return 0;
        }
    }
	return -1;
}

int testGetfilePath() {
	int i=-3;
	DEBUG_LOG("------------testGetFilePath-----\n");
	for ( i = -3 ; i < file_num+3; i++) {
		getFilePath(i);
		DEBUG_LOG("%03d:[%s]\n",i,filePath);
	}
    return 0;
}

imgType getFileType(char *filename) {
    imgType ret;
    char extensions[][6] = { ".JPG" ,  ".PNG", ".BMP" };
    char *type = strrchr(filename,'.');
    if (type) {
        int i;
        for ( i = 0 ; i < 3; i++) {
            if ( strcasecmp(type,extensions[i]) == 0 ) {
                ret = i;
                return ret;
            }
        }
    }
    ret = UNKNOWN; 
    return ret;
}


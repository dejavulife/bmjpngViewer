/*
 * Copyright (c) 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <vita2d.h>

#include "alife_file.h"


/*
 * Symbol of the image.png file
 */
extern unsigned char _binary_alife_png_start;
extern char filePath[MAXLEN];
extern char img_path[MAXLEN];
extern int isSorted;
extern int file_num;
extern int sorting;
extern int sorting_index;
extern char file_names[MAXFILE][MAXLEN];


float xd, yd, pic_xd, pic_yd, pic_x, pic_y, min_pic_x, min_pic_y, width , hight, width_scale,hight_scale, min_scale ;
char str[MAXLEN]= {};
char baseDir[MAXGAMES][MAXLEN];
char configDir[MAXGAMES][MAXLEN];
int totalDirs = 0, selectedDir=0, dirPage=0;
float scale_step = 0.01, scrollSpeed = 16, scrollSlowSpeed = 8; 
float key_next, key_pre, key_next_item, key_pre_item, key_widthFit, key_hightFit, key_pixelFit, key_scrollUp, key_scrollDown, key_scaleChange, key_cancel, key_confirm;
int key_exit, key_Xmove, key_Ymove, key_zoomIn, key_zoomOut, key_help;
int keyDelayCount= 10000, keyDelay_L, keyDelay_R;
SceTouchData touch_old[SCE_TOUCH_PORT_MAX_NUM];
SceTouchData touch_press[SCE_TOUCH_PORT_MAX_NUM];
SceTouchData touch_release[SCE_TOUCH_PORT_MAX_NUM];
SceTouchData touch[SCE_TOUCH_PORT_MAX_NUM];
int touch_press_x, touch_press_y, touch_x, touch_y, touch_release_x, touch_release_y; 
int istouch_press = 0;
int jump_start_x = 30, jump_start_y = 50;
int jump_pagenum = 0 ,jump_selected_line = 0 , jump_selected_col = 0, dojumpfile = 0, jumptofilenum = 0;
int rescanning = 0 ;
int pre_file=0, next_file=0, cur_file = 0, preview_file;
int pre_ready = 0, next_ready = 0;
int need_update ;
int need_help = 1;
char oldfile[1024];
float scale = 1.0f, current_scale = 1.0f;
enum GAMESCREEN gameScreen = MENU;
char errorStr[MAXLEN];

SceCtrlData pad;	
vita2d_pgf *pgf;
vita2d_texture *image = NULL;
vita2d_texture *bg = NULL;
vita2d_texture *image4 = NULL;
vita2d_texture *pic[6];
char picfile[6][MAXLEN];
int prefiles[2],curfiles[2],nextfiles[2];
int font = 0;
int configItem[10] = {-101,-101,-101,-101,-101,-101,-101,-101,-101,-101};
int needSaveSetting = 0;
int X_INVERT = -1, OLD_X_INVERT = -1;
int Y_INVERT = -1, OLD_Y_INVERT = -1;
int isArrowForPage = 0, OLD_isArrowForPage= 0;
float OLD_scrollSpeed = 0;
float testpic_x = SCREEN_HALF_WIDTH, testpic_y = SCREEN_HALF_HIGHT;
time_t now = 0;


__attribute__((format(printf,1,2)))
int debug_log (char* format, ... ) {
	FILE *fp;
	fp = fopen("ux0:data/alife_debug.txt","a");
	if ( fp == NULL ) {
		//printf("open alife_debug.txt failed.\n");
		return 0;
	}
	char str_tmp[MAXLEN];
    int i=0;
    va_list vArgList;                           
    va_start (vArgList, format);                 
    i=vsnprintf(str_tmp, MAXLEN, format, vArgList);  
    va_end(vArgList);                            

    fprintf(fp,"%s", str_tmp);
	fclose(fp);
	return i;
}

int viewbaseDir() {
    int i;
    for ( i=0;i<totalDirs;i++){
        DEBUG_LOG("baseDir[%d]=[%s]\n",i,baseDir[i]);
    }
    return 0;
}

vita2d_texture *loadImageFile (char *file) {
    vita2d_texture *img;
    imgType x = getFileType(file);
    DEBUG_LOG("file[%s], filetype:%d\n",file,x);
    if (x == UNKNOWN) {
        img = NULL;
        return img;
    }
    switch(x) {
        case JPG: 
             img = vita2d_load_JPEG_file(file);
            break;
        case PNG:
            img = vita2d_load_PNG_file(file);
            break;
        case BMP:
            img = vita2d_load_BMP_file(file);
            break;
        default:
            img = NULL;
    }
    DEBUG_LOG("finish loading file[%s]\n",file);
    return img;
}

int fileToConfig() {
    int res = -1;
    float fx = 0 ;
    res = loadConfigFile();
    if (res != 0 ) return -1;
    int  i=0;
    for ( i =0 ;i < 4;i++ ) {
        if ( configItem[i] != -101 ) {
            switch(i) {
                case 0:
                    if ( configItem[i] == 1 || configItem[i] == -1) X_INVERT = configItem[i];
                    break;
                case 1:
                    if ( configItem[i] == 1 || configItem[i] == -1) Y_INVERT = configItem[i];
                    break;
                case 2:
                    if ( configItem[i] == 1 || configItem[i] == -1) isArrowForPage = configItem[i];
                    break;
                case 3:
                    fx = (float)configItem[i];
                    if ( fx >= 10 && fx <= 30)  scrollSpeed = fx;
                    break;
                default:
                    break;
            }
        }
    }
    return 0;
}

int configToFile() {
    int res = -1;
    int  i=0;
    for ( i =0 ;i < 4;i++ ) {
            switch(i) {
                case 0:
                    configItem[i] =  X_INVERT ;
                    break;
                case 1:
                    configItem[i] = Y_INVERT  ;
                    break;
                case 2:
                     configItem[i] = isArrowForPage;
                    break;
                case 3:
                    configItem[i] = (int)scrollSpeed;
                    break;
                default:
                    break;
            }
    }
    res = saveConfigFile();
    return res;
}


int getconfig() {
	FILE *fp;
	fp = fopen("ux0:data/comic_config.txt","r");
	if ( fp == NULL ) {
		DEBUG_LOG("open comic_config.txt failed.\n");
        strcpy(configDir[0],"ux0:comic/");
		return 1;
	}
	int i=0;
	for ( i = 0; i < MAXGAMES; i++ ) {
		fscanf(fp," %s", configDir[i]);
		DEBUG_LOG( "%d: dir[%s]\n",i,configDir[i]);
		fscanf(fp," \n");
		if ( strlen(configDir[i]) <= 0 ) break;
	}
	DEBUG_LOG(" total %d games\n",i);
	fclose(fp);

	return 0;
}


int initAll() {
	//getconfig();
    fileToConfig();
    scrollSlowSpeed = scrollSpeed/2;
    strcpy(configDir[0],"ux0:comic/");
	/* to enable analog sampling */
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
    sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
    pgf = vita2d_load_custom_pgf(FONTPGF);
    if (pgf == NULL) { 
        pgf = vita2d_load_default_pgf(); 
        font = 1;
    }
	/*
	 * Load the statically compiled image.png file.
	 */
	image = vita2d_load_PNG_buffer(&_binary_alife_png_start);
    bg = loadImageFile(BACKGROUNDFILE);
    cur_file = 0;

    strcpy(img_path,configDir[0]);
    DEBUG_LOG("img_path[%s]\n",img_path);
    if ( strlen(img_path) <= 4 ) {
        snprintf(errorStr,MAXLEN, "img_path [%s]",img_path);
    } else { 
        getDirList(img_path);
        gameScreen = MENU;
        viewbaseDir();
    }
	memset(&pad, 0, sizeof(pad));
	need_update=1 ;
    return 0;
}

int clearKeys() {
    key_next=0 , key_pre =0 , key_next_item = 0 , key_pre_item =0 ;
    key_widthFit = 0, key_hightFit = 0, key_pixelFit = 0;
    key_scrollUp = 0, key_scrollDown = 0, key_scaleChange = 0, key_Xmove = 0, key_Ymove = 0;
    key_exit = 0 , key_zoomIn = 0, key_zoomOut = 0;
    key_cancel = 0 , key_confirm = 0, key_help = 0;
    return 0;
}

int totalKeypress() {
    return (key_next + key_pre + key_next_item + key_pre_item +  key_widthFit + key_hightFit + key_pixelFit + key_scrollUp \
          + key_scrollDown + key_scaleChange + key_Xmove + key_Ymove +  key_exit \
          + key_zoomIn + key_zoomOut + key_cancel + key_confirm + key_help);
}

int getinput() {
    clearKeys();
    memcpy(touch_old, touch, sizeof(touch_old)); 
	sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(0, &touch[SCE_TOUCH_PORT_FRONT], 1);
    if ( istouch_press == 0) {
        if ( touch[SCE_TOUCH_PORT_FRONT].reportNum > 0 ) {
            memcpy(touch_press, touch, sizeof(touch_press));
            touch_press_x = touch_press[SCE_TOUCH_PORT_FRONT].report[0].x;
            touch_press_y = touch_press[SCE_TOUCH_PORT_FRONT].report[0].y;            
            istouch_press = 1;
        }
    } else if ( istouch_press == 1 ) {
        if (  touch[SCE_TOUCH_PORT_FRONT].reportNum <= 0 ) {
            memcpy(touch_release, touch_old, sizeof(touch_release));
            touch_release_x = touch_release[SCE_TOUCH_PORT_FRONT].report[0].x;
            touch_release_y = touch_release[SCE_TOUCH_PORT_FRONT].report[0].y;      
            istouch_press = 0;
        }
    }

    int i = 0;
    for(i = 0; i < SCE_TOUCH_MAX_REPORT; i++) {
        if ( i < touch[SCE_TOUCH_PORT_FRONT].reportNum ) {
            touch_x = touch[SCE_TOUCH_PORT_FRONT].report[i].x;
            touch_y = touch[SCE_TOUCH_PORT_FRONT].report[i].y;
        }
    }
    if (pad.buttons & SCE_CTRL_START) {
        key_exit = 1; 
        return 0;
    }
    if (pad.buttons & SCE_CTRL_SELECT) key_help = 1;
    if (pad.buttons & SCE_CTRL_UP)  key_scrollUp = 1;
    if (pad.buttons & SCE_CTRL_DOWN)  key_scrollDown = 1;
    if (pad.buttons & SCE_CTRL_LEFT)  key_pre = 1;
    if (pad.buttons & SCE_CTRL_RIGHT)  key_next = 1;
    if (pad.buttons & SCE_CTRL_TRIANGLE) key_hightFit = 1;
    if (pad.buttons & SCE_CTRL_SQUARE) key_widthFit = 1;
    if (pad.buttons & SCE_CTRL_CIRCLE) { key_pixelFit = 1; key_confirm = 1;}
    if (pad.buttons & SCE_CTRL_CROSS) key_cancel = 1;
    key_Xmove = pad.lx ;
    key_Ymove = pad.ly ;
    if ( pad.ry > 140 ) key_zoomIn=1;
    if ( pad.ry < 120 ) key_zoomOut = 1;
    if (pad.buttons & SCE_CTRL_LTRIGGER) key_pre_item = 1;
    if (pad.buttons & SCE_CTRL_RTRIGGER) key_next_item = 1;
    return 0;
}

int doMenuInnerLogic() {
    if (key_next||key_next_item) {
        dirPage++;
        if (dirPage > (totalDirs-1)/10 ) dirPage = (totalDirs-1)/10; 
        else {
            selectedDir += 10;
            if ( selectedDir >= totalDirs ) {
                selectedDir= totalDirs-1;
            }
        }
    }
    if (key_pre|| key_pre_item) {
        dirPage--;
        if (dirPage < 0) dirPage = 0;
        else selectedDir -= 10;
    }

    if (key_scrollDown) {
        selectedDir++;
        if ( selectedDir % 10 == 0 ) {
            dirPage++;
        }
    }
    if (selectedDir >= totalDirs ) {
        selectedDir = 0;
        dirPage = 0;
    }
    if (key_scrollUp) {
        selectedDir--;
        if ( selectedDir % 10 == 9 ) {
            dirPage--;
        }
    }
    if (selectedDir < 0 ) {
        selectedDir = totalDirs-1;
        dirPage = (totalDirs-1)/10;
    }

    // key_pixelFit is CIRCLE key
    if (key_pixelFit) {
        
        strcpy(img_path,configDir[0]);
        if ( img_path[strlen(img_path)-1] != '/') strcat(img_path,"/");
        strcat(img_path,baseDir[selectedDir]); 
        DEBUG_LOG("selectedDir[%s]\n",img_path);
        pre_file = 0;
        next_file = 0;
        cur_file = loadCurrentIndex();
        if (cur_file == -1 ) cur_file = 0;
		gameScreen = PLAY;
        DEBUG_LOG(" selected time is %ld\n", TIMENOW);
        return 0;
    }
    if (key_hightFit) {
        gameScreen = SCANNING;
    }
	if (key_widthFit) gameScreen = INFO;
    if (key_cancel) {
        testpic_x = SCREEN_HALF_WIDTH, testpic_y = SCREEN_HALF_HIGHT;
        OLD_X_INVERT = X_INVERT, OLD_Y_INVERT = Y_INVERT, OLD_isArrowForPage = isArrowForPage, 
        OLD_scrollSpeed = scrollSpeed;
        gameScreen = OPTION;
        need_update=1;
        sceKernelDelayThread(200*1000); 
        key_cancel = 0;
    }
    return 0;
}

int viewComicInnerLogic() {
    if (key_help) {
        if ( need_help == 1 ) need_help = 0;
        else need_help = 1;
        sceKernelDelayThread(200*1000); 
    }
    if (key_cancel) {
        gameScreen = MENU;
        vita2d_wait_rendering_done();
        vita2d_free_texture(image4);
        saveCurrentIndex(cur_file);
        cur_file = 0;
        image4 = NULL;
    } else if ( istouch_press ) {
        gameScreen = JUMP;
        touch_release_x = 0;
        touch_release_y = 0;
        return 0;
    }
    if (key_scrollUp) pic_y -= Y_INVERT * scrollSpeed;
    if (key_scrollDown) pic_y += Y_INVERT * scrollSpeed;
    if (!isArrowForPage ) {
        if ( key_pre) pic_x -= X_INVERT * scrollSpeed;
        if ( key_next ) pic_x += X_INVERT * scrollSpeed; 
    }
	if (key_hightFit)  {
            scale =  hight_scale;
            pic_x = 0;
            pic_y = 0;
            need_update = 1;
        }

	if (key_widthFit)  {
            scale =  width_scale;
            pic_x = 0;
            need_update = 1;
        }
	if (key_pixelFit)  scale = 1.0f;
/*
        if (key_Xmove < 120 ) pic_x -= ( 120 - key_Xmove )/2;
        if (key_Xmove > 140 ) pic_x += ( key_Xmove - 140 )/2;
        if (key_Ymove < 120 ) pic_y -= ( 120 - key_Ymove )/2;
        if (key_Ymove > 140 ) pic_y += ( key_Ymove - 140 )/2;
*/
        if (key_Xmove < 120 ) pic_x -= X_INVERT * scrollSlowSpeed; 
        if (key_Xmove > 140 ) pic_x += X_INVERT * scrollSlowSpeed;
        if (key_Ymove < 120 ) pic_y -= Y_INVERT * scrollSlowSpeed;
        if (key_Ymove > 140 ) pic_y += Y_INVERT * scrollSlowSpeed;

        if (key_zoomIn) scale -= scale_step;
        if (key_zoomOut ) scale += scale_step;
        if ( scale < min_scale ) scale = min_scale;
        if (( isArrowForPage && key_pre ) || key_pre_item) {
            pre_file = 1;
        }
        
        if (( isArrowForPage && key_next ) || key_next_item) { 
            next_file = 1;
        }
        /* below is fixing wrong pic_x or pic_y value */
        if ( pic_x > 50 ) pic_x = 50;
        if ( pic_y > 50 )  { 
            pic_y = 50;
        }
        if (  width * current_scale > 960 ) { 
            min_pic_x = 960.0 - width * current_scale - 50.0;
        } else { 
            min_pic_x = ( 960.0 - width * current_scale ) / 2 ;
            pic_x = min_pic_x;
        }
        if ( hight * current_scale > 544 ) {
            min_pic_y = 544.0 - hight * current_scale - 50.0;
        } else {
            min_pic_y = ( 544.0 - hight * current_scale ) / 2;
        }
        if ( pic_x < min_pic_x ) pic_x = min_pic_x;
        if ( pic_y < min_pic_y ) { 
            pic_y = min_pic_y;
        }
        if ( pic_x != pic_xd || pic_y != pic_yd ) {
            pic_xd = pic_x;
            pic_yd = pic_y;
            need_update = 1;
        }
        /* make sure update only when needed */
        /* if you push ADXO buttons, screen need to update.*/
        if ( scale != current_scale) need_update = 1;
        if ( scale > current_scale ) {
            current_scale += scale_step;
            if ( current_scale > scale ) current_scale = scale;
        } else if (scale < current_scale) {
            current_scale -= scale_step;
            if ( current_scale < scale ) current_scale = scale;
        } else {
            
        }
        return 0;
}

int jumpInnerLogic() {
    if (key_help) {
        if ( need_help == 1 ) need_help = 0;
        else need_help = 1;
        sceKernelDelayThread(200*1000);        
    }
    if (key_cancel) {
        gameScreen = PLAY;
        sceKernelDelayThread(1000*1000);
    }
    if (key_confirm) {
        int tmp=0;
        tmp = jump_pagenum * 50 + jump_selected_line * 10 + jump_selected_col ;
        if (tmp < file_num) {
            jumptofilenum = tmp;
            dojumpfile = 1;
            gameScreen = PLAY;
        }
    }
    jump_selected_col = ( (touch_x / 2) - jump_start_x ) / 90;
    jump_selected_line = ( (touch_y /2 ) - jump_start_y ) / 90; 
    if (key_pre ) {
        jump_pagenum--;
    }
    if ( key_pre_item ) jump_pagenum -= 5;
    if (key_next ) { 
        jump_pagenum++;
    }
    if ( key_next_item ) jump_pagenum += 5;
    if (jump_pagenum < 0 ) jump_pagenum = 0;
    if (jump_pagenum > (file_num/50)) jump_pagenum = file_num/50;
    return 0;
}

int scanInnerLogic() {
    if (rescanning == 0 ) {
        rescanning =1;
        strcpy(img_path,configDir[0]);
        if ( img_path[strlen(img_path)-1] != '/') strcat(img_path,"/");
        strcat(img_path,baseDir[selectedDir]); 
        DEBUG_LOG("scan[%s]",img_path);
        if ( strcmp(img_path, configDir[0]) == 0 ) {
            need_update = 1;
            DEBUG_LOG("scan logic , not selected anything");
            rescanning = 0;
            gameScreen = MENU;
            return 0;
        }
        DEBUG_LOG("re scan selectedDir[%s]\n",img_path);
        DEBUG_LOG("re scan start time is %ld\n", TIMENOW);
        SceUID thid = sceKernelCreateThread("sorting_thread", (SceKernelThreadEntry)resetDirInfos, 0x40, 0x100000, 0, 0, NULL); 
        if (thid >= 0)
            sceKernelStartThread(thid, 0, NULL);
        need_update=1;
        isSorted = 0;
    }
    if ( rescanning == 1) need_update = 1;
    if ( isSorted == 1 && sorting == 0) {
        DEBUG_LOG("sorting is done! time %ld\n",TIMENOW);
        rescanning = 0;
        gameScreen = PLAY;
        cur_file = 0;
    }
    return 0;
}

int softinfoInnerLogic() {
    if (key_help) {
        if ( need_help == 1 ) need_help = 0;
        else need_help = 1;
        sceKernelDelayThread(200*1000); 
    }
    if (key_cancel) {
        gameScreen = MENU;
    }
    return 0;
}

int optionInnerLogic() {
    if (key_help) {
        if ( need_help == 1 ) need_help = 0;
        else need_help = 1;
        sceKernelDelayThread(200*1000);        
    }
    if (needSaveSetting) {
        if (key_cancel) { 
            needSaveSetting=0;
            X_INVERT = OLD_X_INVERT ;
            Y_INVERT = OLD_Y_INVERT ;
            isArrowForPage = OLD_isArrowForPage ;
            scrollSpeed = OLD_scrollSpeed ;
            scrollSlowSpeed = scrollSpeed/2;
            gameScreen = MENU;
            DEBUG_LOG("not save config");
            sceKernelDelayThread(500*1000);
            return 0;
        } else if ( key_confirm) {
            DEBUG_LOG("save config to File");
            configToFile();
            needSaveSetting=0;
            gameScreen = MENU;
            sceKernelDelayThread(500*1000);
            return 0;
        }
        return 0;
    }
    if (key_cancel) {
        if (   OLD_X_INVERT != X_INVERT \
            || OLD_Y_INVERT != Y_INVERT \
            || OLD_isArrowForPage != isArrowForPage \
            || OLD_scrollSpeed != scrollSpeed ) {
           needSaveSetting=1; 
           sceKernelDelayThread(500*1000);
        } else {
            gameScreen = MENU;
            sceKernelDelayThread(500*1000);
            return 0;
        }
        
    }
    if ( key_confirm) {
        if ( isArrowForPage) isArrowForPage = 0;
        else isArrowForPage = 1;
        sceKernelDelayThread(200*1000);
    }
    
    if ( key_widthFit) {
        X_INVERT *= -1;
        sceKernelDelayThread(200*1000);
    }
    if ( key_hightFit) {
        Y_INVERT *= -1;
        sceKernelDelayThread(200*1000);
    }
    if ( key_pre_item) {
        scrollSpeed -= 2;
        sceKernelDelayThread(100*1000);
    }
    if ( key_next_item) {
        scrollSpeed += 2;
        sceKernelDelayThread(100*1000);
    }
    if ( scrollSpeed < MINSCROLLSPEED ) scrollSpeed = MINSCROLLSPEED;
    if ( scrollSpeed > MAXSCROLLSPEED ) scrollSpeed = MAXSCROLLSPEED;
    scrollSlowSpeed = scrollSpeed/2;
    if (key_scrollUp) testpic_y -= Y_INVERT * scrollSpeed;
    if (key_scrollDown) testpic_y += Y_INVERT * scrollSpeed;
    if (!isArrowForPage ) {
        if ( key_pre) testpic_x -= X_INVERT * scrollSpeed;
        if ( key_next ) testpic_x += X_INVERT * scrollSpeed; 
    }
    if (key_Xmove < 120 ) testpic_x -= X_INVERT * scrollSlowSpeed; 
    if (key_Xmove > 140 ) testpic_x += X_INVERT * scrollSlowSpeed;
    if (key_Ymove < 120 ) testpic_y -= Y_INVERT * scrollSlowSpeed;
    if (key_Ymove > 140 ) testpic_y += Y_INVERT * scrollSlowSpeed;
    return 0;
}

int innerLogic() {
    int c = 0;
    c = totalKeypress() ;
    if ( c > 0 ) need_update = 1;
    switch(gameScreen) {
        case MENU:
            doMenuInnerLogic();
			break;
        case INFO:
            softinfoInnerLogic();
            break;
        case PLAY:
            viewComicInnerLogic();
            break;
        case JUMP:
            jumpInnerLogic();
            break;
        case SCANNING:
            scanInnerLogic();
            break;
        case OPTION:
            optionInnerLogic();
            break;
    }
    return 0;
}

int showDebugInfo() {
    if (strlen(errorStr) > 0) vita2d_pgf_draw_text(pgf,400, 30,RGBA8(255,0,0,255),1.0f,errorStr);
    sprintf(str, "[%05d/%05d] %s",cur_file+1,file_num,filePath);
    vita2d_pgf_draw_text(pgf, 0, 30, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, "W_scale = %.2f H_scale = %.2f Cur_scale = %.2f , Scale = %.2f\n pic_x = %.2f, pic_y = %.2f ", width_scale, hight_scale, current_scale, scale, pic_xd,pic_yd);
    vita2d_pgf_draw_text(pgf, 0, 60, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, " sorted = %d ", isSorted);
    vita2d_pgf_draw_text(pgf, 0, 90, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, " img_path  [%s] ", img_path);
    vita2d_pgf_draw_text(pgf, 0, 100, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, " touch press [%d,%d] touch [%d,%d] touch release [%d,%d]", touch_press_x,touch_press_y,touch_x,touch_y, touch_release_x, touch_release_y);
    vita2d_pgf_draw_text(pgf, 0, 130, RGBA8(0,255,0,255), 1.0f, str);
    return 0;
}

int showHelp() {
    switch(gameScreen) {
    	case MENU:
            sprintf(str, " "); 
            break;
		case INFO:
            sprintf(str, "X键返回"); 
            break;
        case PLAY:
            sprintf(str, "触屏跳页/左杆移图/右杆缩放/左右LR翻页/三角全图/方框适宽/圆圈等倍/X返回/START退出");
            break;
        case JUMP:
             sprintf(str, "左右翻1页，L/R跳5页");
            break;
        case SCANNING:
            sprintf(str, " "); 
            break;
        case OPTION:
             sprintf(str, "按方向键或左摇杆移动图片, X返回");
             break;
    }
    vita2d_pgf_draw_text(pgf, 20, 520, RGBA8(0,255,0,255), 1.0f, str);
    return 0;
}

int scanView() {
    vita2d_start_drawing();
	vita2d_clear_screen();
    sprintf(str, "正在扫描目录[%s] ", img_path);
    vita2d_pgf_draw_text(pgf, 100, 100, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, "处理中 [%04d/%04d] ", sorting_index+1, file_num);
    vita2d_pgf_draw_text(pgf, 100, 200, RGBA8(0,255,0,255), 1.0f, str);
    vita2d_end_drawing();
	vita2d_swap_buffers();
    return 0;
}

int doShowSoftInfo() {
    int line_start_y = 50 , linehight_step = 50, i = 0;
 // cleaning screen.
	vita2d_start_drawing();
	vita2d_clear_screen();
    sprintf(str, "1. 本软件目标是浏览中小图片。摇杆的移图速度是方向键的一半。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "    图片尺寸在2000*2000以内，大小不超过2M。太大的图片打开慢。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "2. 由第1点，打开太大的图片，可能会引起本软件工作不正常，后果自负。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "    如异常退出，GPU驱动崩溃等，需要长按电源重启。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "3. 本软件是用libvita2d库打开图片，有些图片打开或显示会出问题。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "    这种情况下，只能手工删除有问题的图片。删除办法见4。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "4. 使用vitaShell打开，会异常退出的图片，必须删除。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    snprintf(str,1024,"5. 背景图放在ux0:comic/bg.png 分辨率960*544");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    snprintf(str,1024,"6. 下载地址: https://github.com/dejavulife/bmjpngViewer/releases");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    snprintf(str,1024,"7. 本软件未经完全测试，不作任何保证。后果自负。");
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(255,0,0,255), 1.5f, str);
    if (need_help) showHelp();
    vita2d_end_drawing();
	vita2d_swap_buffers();
    return 0;
}

int doShowMenu() {
    int i=0,index=0 ;
    float bold = 1.0;
    char str[1024];
    // cleaning screen.
	vita2d_start_drawing();
	vita2d_clear_screen();
    if (bg != NULL) vita2d_draw_texture(bg,0,0);
    if (totalDirs <= 0 ) {
		if ( image != NULL) vita2d_draw_texture_rotate(image, 940/2, 544/2, 0.0);
         vita2d_pgf_draw_text(pgf, 10, 100, RGBA8(0,255,0,255), 2.0f, "目前找不到漫画，请把漫画的文件夹放到ux0:/comic/目录下。");
    } else {
        snprintf(str,1024," 共%d个目录 当前页[%d/%d]",totalDirs, dirPage+1,(totalDirs-1)/10+1);
        vita2d_pgf_draw_text(pgf, 20, 30, RGBA8(0,255,0,255), 1.0f, str);
        snprintf(str,1024,"使用此软件属用户个人行为，一切后果自行承担。NO GUARANTEE！");
        vita2d_pgf_draw_text(pgf, 300, 30, RGBA8(255,0,0,255), 1.0f, str);
        for ( i = 0; i < 10; i++ ) {
            index = i + dirPage*10;
            bold = 1.0 ; 
            if ( index == selectedDir) bold = 2.0; 
            if ( index < totalDirs) vita2d_pgf_draw_text(pgf, 50, 40*(i+1)+30, RGBA8(0,255,0,255), bold, baseDir[index]);
        }
    }
    sprintf(str, "初次进入目录请等待扫描。目录文件有变化，可按三角重扫。");
    vita2d_pgf_draw_text(pgf, 20, 500, RGBA8(0,255,0,255), 1.0f, str);
    sprintf(str, "SELECT打开关闭帮助/LR方向键选择/圆圈进入/方框说明/X键设置/START退出");
    vita2d_pgf_draw_text(pgf, 20, 520, RGBA8(0,255,0,255), 1.0f, str);
    if (font == 1 ) {
        snprintf(str,1024,"如果文字xian示有wen ti，fu制(copy) ux0:app/COMICVIEW/font.pgf 到 ux0:data/font.pgf");
        vita2d_pgf_draw_text(pgf, 20, 540, RGBA8(255,0,0,255), 1.0f, str);
    }
    vita2d_end_drawing();
	vita2d_swap_buffers();
    sceKernelDelayThread(100*1000);
    return 0;
}


int viewComic() {
    int res=0;
    if (dojumpfile) {
        dojumpfile = 0;
        res = getFilePath(jumptofilenum);
        if (res == -1){
            return 0 ;
        }
        strcpy(str,filePath);
        if ( image4 != NULL ) {
            vita2d_wait_rendering_done();
            vita2d_free_texture(image4);
        }
        image4 = loadImageFile(str);
        if( image4 != NULL) {
            strcpy(oldfile,str);
            cur_file = jumptofilenum;
            width = vita2d_texture_get_width(image4);
            hight = vita2d_texture_get_height(image4);
            width_scale = ( 960.0 / (float)width);
            hight_scale = ( 544.0 / (float)hight); 
            min_scale = ( width_scale < hight_scale ? width_scale : hight_scale ) - 2 * scale_step;
            if ( min_scale > 1.0 ) min_scale = 1.0;
            current_scale = scale;
            pic_x = 0 ;
            pic_y = 0 ;
            need_update = 1;
        } else {
            //image4 = loadImageFile(oldfile); 
            gameScreen = SCANNING;
        }
    }
    if ( image4 == NULL) {
        res = readFileList(img_path);
        DEBUG_LOG(" finish readFile time is %ld\n", TIMENOW);
        if ( res == -1 ) {
            gameScreen = SCANNING;
            return 0;
        }
        res = getFilePath(cur_file);
        if (res != 0 ) sprintf(errorStr , "file index not finish, cant open image.");
        else {
            strcpy(str,filePath);
            // DEBUG_LOG("will load [%s] file\n",str);
            if ( strlen(str) > 0 ) {
                image4 = loadImageFile(str);
                DEBUG_LOG(" finish loadImageFile time is %ld\n", TIMENOW);
                strcpy(oldfile,str);
                width = vita2d_texture_get_width(image4);
                hight = vita2d_texture_get_height(image4);
                width_scale = ( 960.0 / (float)width);
                hight_scale = ( 544.0 / (float)hight); 
                min_scale = ( width_scale < hight_scale ? width_scale : hight_scale ) - 2 * scale_step; 
                if ( min_scale > 1.0 ) min_scale = 1.0;
                current_scale = width_scale > 1 ? 1 : width_scale;
            }
        }
    }
    if (pre_file || next_file) {
            if ( pre_file)  preview_file = preFile(cur_file);
            if (next_file)  preview_file = nextFile(cur_file);
            if ( preview_file == -1 ) {
                    pre_file = 0 ;
                    next_file = 0;
                    return 0;
            }
            res = getFilePath(preview_file);
            if (res == -1){
                pre_file = 0 ;
                next_file = 0;
                return 0 ;
            }
            strcpy(str,filePath);
            vita2d_wait_rendering_done();
            vita2d_free_texture(image4);
            image4 = loadImageFile(str);
            if( image4 != NULL) {
                strcpy(oldfile,str);
                cur_file = preview_file;
                width = vita2d_texture_get_width(image4);
                hight = vita2d_texture_get_height(image4);
                width_scale = ( 960.0 / (float)width);
                hight_scale = ( 544.0 / (float)hight); 
                min_scale = ( width_scale < hight_scale ? width_scale : hight_scale ) - 2 * scale_step;
                if ( min_scale > 1.0 ) min_scale = 1.0;
                current_scale = scale;
                pic_x = 0 ;
                pic_y = 0 ;
                need_update = 1;
            } else {
                //image4 = loadImageFile(oldfile); 
                gameScreen = SCANNING;
            }
            pre_file = 0;
            next_file = 0;
        }
        // cleaning screen.
	vita2d_start_drawing();
	vita2d_clear_screen();
        // drawing 
        if (image4 != NULL) vita2d_draw_texture_scale(image4,pic_xd,pic_yd,current_scale,current_scale);
        
#ifdef DEBUG
        showDebugInfo();
#endif
        if (need_help) {
            if ( width > 2000 || hight > 2000 ) {
                sprintf(str, "图片尺寸:%gx%g",width,hight);
                vita2d_pgf_draw_text(pgf, 300, 30, RGBA8(255,0,0,255), 1.0f, str);
            }
		showHelp();
        sprintf(str, "[%05d/%05d]",cur_file+1,file_num);
        vita2d_pgf_draw_text(pgf, 800, 500, RGBA8(0,255,0,255), 1.0f, str);
        sprintf(str, "%s / %s",baseDir[selectedDir],file_names[cur_file]);
        vita2d_pgf_draw_text(pgf, 20, 540, RGBA8(0,255,0,255), 1.0f, str);
        sprintf(str, "%d%%",(int)(current_scale*100));
        vita2d_pgf_draw_text(pgf, 800, 520, RGBA8(0,255,0,255), 1.0f, str);
    }
	vita2d_end_drawing();
	vita2d_swap_buffers();
    return 0;
}

  /*  pageIndex = index / 50 ;
     *  cur_num = index % 50;
     *  line_num = cur_num / 10;
     *  colum_num = cur_num % 10;
     *  x = letter_adjust_x + start_x + ( line_num * 90);
     *  y = letter_adjust_y + start_y + ( colum_num * 90);
     */
int drawPageNum(int pageNum, int maxNum) {
    int line = 0, col = 0, picNum_x = 0 , picNum_y = 0, letter_adjust_x = 20, letter_adjust_y = 50;
    int mini_adjust_x = 0, mini_adjust_y = 0;
    for ( line = 0 ; line < 5; line++ ) {
        for ( col = 0 ; col < 10; col++) {
            if ( maxNum <= line*10 + col) break;
            sprintf(str, "%3d", pageNum*50 + line*10 + col+1);
            picNum_x = letter_adjust_x + jump_start_x + ( col * 90);
            picNum_y = letter_adjust_y + jump_start_y + ( line * 90);
            if ( col == jump_selected_col && line == jump_selected_line) {
                if ( pageNum >= 20 ) {
                    mini_adjust_x = -18;
                    mini_adjust_y = 5;
                } else {
                    mini_adjust_x = -5;
                    mini_adjust_y = 5;
                }
                vita2d_pgf_draw_text(pgf, picNum_x+mini_adjust_x, picNum_y+mini_adjust_y, RGBA8(255,0,0,255), 1.5f, str);
            } else {
                vita2d_pgf_draw_text(pgf, picNum_x, picNum_y, RGBA8(0,255,0,255), 1.0f, str);
            }
        }
        if ( maxNum <= line*10 + col) break;
    }
    return 0;
  
}


int jumpView() {
    // cleaning screen.
	vita2d_start_drawing();
	vita2d_clear_screen();
    sprintf(str, " 共%d页 [%d/%d]  O确认  X返回 用手点数字", file_num, jump_pagenum+1, file_num/50+1);
    vita2d_pgf_draw_text(pgf, 20, 30, RGBA8(0,255,0,255), 1.0f, str);
#ifdef DEBUG
    sprintf(str, " 共%d页 [%d/%d]。现选中 line: %d, col:%d ", file_num, jump_pagenum+1, file_num/50+1, jump_selected_line,jump_selected_col);
    vita2d_pgf_draw_text(pgf, 10, 50, RGBA8(0,255,0,255), 1.0f, str);
#endif
    int i =0 ;
    for ( i =0; i < 6; i++ ) {
        vita2d_draw_line(jump_start_x, 90*i+jump_start_y, jump_start_x+900, 90*i + jump_start_y, RGBA8(0 , 0, 255, 255));
    }
    for ( i = 0; i < 11; i++ ) {
        vita2d_draw_line(90*i+jump_start_x, jump_start_y, 90*i+jump_start_x, jump_start_y+450, RGBA8(0 , 0, 255, 255));
    }
    if ( jump_pagenum < file_num/50 ) {
        drawPageNum(jump_pagenum, 50);
    } else {
        drawPageNum(jump_pagenum, file_num % 50 );
    }
#ifdef DEBUG
    sprintf(str, " touch press [%d,%d] touch [%d,%d] touch release [%d,%d]", touch_press_x,touch_press_y,touch_x,touch_y, touch_release_x, touch_release_y);
    vita2d_pgf_draw_text(pgf, 10, 550, RGBA8(0,255,0,255), 1.0f, str);
#endif
    if (need_help) showHelp();
	vita2d_end_drawing();
	vita2d_swap_buffers();
    sceKernelDelayThread(100*1000);    
    return 0;
}

int optionView() {
    int line_start_y = 50 , linehight_step = 50, i = 0;
    char yes[] = "是", no[] = "否";
    // cleaning screen.
	vita2d_start_drawing();
	vita2d_clear_screen();
    
    // drawing
    if (!needSaveSetting) {
        if (image != NULL) vita2d_draw_texture_rotate(image,testpic_x,testpic_y,0.0);
    }
    sprintf(str, "方框:  X轴 是否反转 [%s]", (X_INVERT > 0 ?yes:no));
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "三角:  Y轴 是否反转 [%s]", (Y_INVERT > 0 ?yes:no));
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "L/R:  移动速度 [%g]", scrollSpeed);
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    sprintf(str, "圆圈: 左右作为翻页 [%s]", isArrowForPage > 0 ? yes:no);
    vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    if ( needSaveSetting ) {
        sprintf(str, "----------------------------------");
        vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
        sprintf(str, "是否保存修改？ O 保存   X 放弃");
        vita2d_pgf_draw_text(pgf, 20, line_start_y + linehight_step * i++, RGBA8(0,255,0,255), 1.5f, str);
    }
	if(need_help) showHelp();
    vita2d_end_drawing();
	vita2d_swap_buffers();
    return 0;
}

int doGraphic() {
    need_update = 0;
    switch(gameScreen) {
        case MENU:
            doShowMenu();
            break;
        case INFO:
            doShowSoftInfo();
            break;
        case PLAY:
            viewComic();
            break;
        case JUMP:
            jumpView();
            break;
        case SCANNING:
            scanView();
            break;
        case OPTION:
            optionView();
            break;
    }
    return 0;
}


int graphicDestroy() {
	/*
	 * vita2d_fini() waits until the GPU has finished rendering,
	 * then we can free the assets freely.
	 */
	vita2d_fini();
	vita2d_free_texture(image);
	if (bg!= NULL) vita2d_free_texture(bg);
	vita2d_free_texture(image4);
	vita2d_free_pgf(pgf);
    return 0;

}

int main()
{
    initAll();
	while (1) {
        getinput();	
		if (key_exit)
			break;
        // innerLogic update things, and if graphic needed to update, need_update is set to 1.
        innerLogic();	
        if (!need_update) continue;
        doGraphic(); 
	}
    graphicDestroy();
	sceKernelExitProcess(0);
	return 0;
}

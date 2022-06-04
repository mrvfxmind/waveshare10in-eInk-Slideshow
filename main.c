#include <time.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "../lib/e-Paper/EPD_IT8951.h"
#include "../lib/GUI/GUI_Paint.h"
#include "../lib/Config/Debug.h"
#include "../lib/Config/DEV_Config.h"
#include "../lib/GUI/GUI_BMPfile.h"
#include <math.h>
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()
#include <string.h>
#include <dirent.h>
#include <stdio.h>

#define __EXAMPLE__
#define Enhance false
#define USE_Factory_Test false
#define USE_Normal_Demo true
#define USE_Touch_Panel false
// 1 bit per pixel, which is 2 grayscale
#define BitsPerPixel_1 1
// 2 bit per pixel, which is 4 grayscale 
#define BitsPerPixel_2 2
// 4 bit per pixel, which is 16 grayscale
#define BitsPerPixel_4 4
// 8 bit per pixel, which is 256 grayscale, but will automatically reduce by hardware to 4bpp, which is 16 grayscale
#define BitsPerPixel_8 8


UBYTE *Refresh_Frame_Buf = NULL;
UBYTE *Panel_Frame_Buf = NULL;
UBYTE *Panel_Area_Frame_Buf = NULL;
bool Four_Byte_Align = false;

extern int epd_mode;
extern UWORD VCOM;
extern UBYTE isColor;
extern UBYTE *Refresh_Frame_Buf;
extern bool Four_Byte_Align;


UWORD VCOM = 2510;

IT8951_Dev_Info Dev_Info;
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	//0: no rotate, no mirror
					//1: no rotate, horizontal mirror, for 10.3inch
					//2: no totate, horizontal mirror, for 5.17inch
					//3: no rotate, no mirror, isColor, for 6inch color
					
void  Handler(int signo){
    Debug("\r\nHandler:exit\r\n");
    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf = NULL;
    }
    if(Panel_Frame_Buf != NULL){
        free(Panel_Frame_Buf);
        Debug("free Panel_Frame_Buf\r\n");
        Panel_Frame_Buf = NULL;
    }
    if(Panel_Area_Frame_Buf != NULL){
        free(Panel_Area_Frame_Buf);
        Debug("free Panel_Area_Frame_Buf\r\n");
        Panel_Area_Frame_Buf = NULL;
    }
    if(bmp_src_buf != NULL){
        free(bmp_src_buf);
        Debug("free bmp_src_buf\r\n");
        bmp_src_buf = NULL;
    }
    if(bmp_dst_buf != NULL){
        free(bmp_dst_buf);
        Debug("free bmp_dst_buf\r\n");
        bmp_dst_buf = NULL;
    }
    Debug("Going to sleep\r\n");
    EPD_IT8951_Sleep();
    DEV_Module_Exit();
    exit(0);
}



/******************************************************************************
function: Change direction of display, Called after Paint_NewImage()
parameter:
    mode: display mode
******************************************************************************/

static void Epd_Mode(int mode)
{
	if(mode == 3) {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_NONE);
		isColor = 1;
	}else if(mode == 2) {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_HORIZONTAL);
	}else if(mode == 1) {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_HORIZONTAL);
	}else {
		Paint_SetRotate(ROTATE_0);
		Paint_SetMirroring(MIRROR_NONE);
	}
}

/******************************************************************************
function: Change image for slide show
******************************************************************************/

UBYTE imageDisplaySlideShow(UWORD Panel_Width, UWORD Panel_Height, UDOUBLE Init_Target_Memory_Addr, UBYTE BitsPerPixel){
    startShow:
        system("cls");


    char imageFolderDir[25] = "/home/8x10frame/Pictures";
    /*****************************************************************************
    25 is the number of characters in the directory path + 1
    *****************************************************************************/
    DIR* dir = ( opendir(imageFolderDir));
    if (dir == NULL) {
        return 1;
    }

    struct dirent* entity;
    entity = readdir(dir);

    /*****************************************************************************
    While loop to got to folder, slect image, display, repeat
    *****************************************************************************/
    
    while (entity !=NULL){
        time_t cur_time;
        char* cur_t_string;
        cur_time = time(NULL);
        cur_t_string = ctime(&cur_time);
        printf("\nThe Current time is : %s \n", cur_t_string);
        

        if (entity->d_type == DT_REG){


        
    


            UWORD WIDTH;
            if(Four_Byte_Align == true){
                WIDTH  = Panel_Width - (Panel_Width % 32);
            }else{
                WIDTH = Panel_Width;
            }
            UWORD HEIGHT = Panel_Height;

            UDOUBLE Imagesize;

            Imagesize = ((WIDTH * BitsPerPixel % 8 == 0)? (WIDTH * BitsPerPixel / 8 ): (WIDTH * BitsPerPixel / 8 + 1)) * HEIGHT;
            if((Refresh_Frame_Buf = (UBYTE *)malloc(Imagesize)) == NULL) {
                Debug("Failed to apply for black memory...\r\n");
                return 0;
            }

        
            
            

            Paint_NewImage(Refresh_Frame_Buf, WIDTH, HEIGHT, 0, BLACK);
            Paint_SelectImage(Refresh_Frame_Buf);
            Epd_Mode(epd_mode);
            Paint_SetBitsPerPixel(BitsPerPixel);
            Paint_Clear(WHITE);

        


            printf("%s/%s\n",imageFolderDir,entity->d_name);

            char Path[300];
            //sprintf(Path,"./pic/%dx%d_0.bmp", WIDTH, HEIGHT);
            sprintf(Path,"%s/%s",imageFolderDir,entity->d_name);

            GUI_ReadBmp(Path, 0, 0);


            EPD_IT8951_4bp_Refresh(Refresh_Frame_Buf, 0, 0, WIDTH,  HEIGHT, false, Init_Target_Memory_Addr,false);
            
            if(Refresh_Frame_Buf != NULL){
                free(Refresh_Frame_Buf);
                Refresh_Frame_Buf = NULL;
            }

            DEV_Delay_ms(5000); //5 sec
            //DEV_Delay_ms(300000); //5 min
            //DEV_Delay_ms(1.8e+6); //30 min
            //DEV_Delay_ms(3.6e+6); //1 hr
            //DEV_Delay_ms(8.64e+7); //1 day
                      
            
        }
        entity = readdir(dir);
    }

     
    goto startShow; // repeat the fuction
    

    return 0;
}





/******************************************************************************
MAIN ACTIVATION FOR FILE
******************************************************************************/

int main(int argc, char *argv[])
{
    //Exception handling:ctrl + c
    signal(SIGINT, Handler);
    
    

    if (argc < 2){
        Debug("Please input VCOM value on FPC cable!\r\n");
        Debug("Example: sudo ./epd -2.51\r\n");
        exit(1);
    }
	if (argc != 3){
		Debug("Please input e-Paper display mode!\r\n");
		Debug("Example: sudo ./epd -2.51 0 or sudo ./epd -2.51 1\r\n");
		Debug("Now, 10.3 inch glass panle is mode1, else is mode0\r\n");
		Debug("If you don't know what to type in just type 0 \r\n");
		exit(1);
    }

    //Init the BCM2835 Device
    if(DEV_Module_Init()!=0){
        return -1;
    }

    double temp;
    sscanf(argv[1],"%lf",&temp);
    VCOM = (UWORD)(fabs(temp)*1000);
    Debug("VCOM value:%d\r\n", VCOM);
	sscanf(argv[2],"%d",&epd_mode);
    Debug("Display mode:%d\r\n", epd_mode);
    Dev_Info = EPD_IT8951_Init(VCOM);

#if(Enhance)
    Debug("Attention! Enhanced driving ability, only used when the screen is blurred\r\n");
    Enhance_Driving_Capability();
#endif

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    A2_Mode = 6;
 
    Debug("A2 Mode:%d\r\n", A2_Mode);

	EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);


/******************************************************************************************
Called function to display slideshow
******************************************************************************************/

imageDisplaySlideShow(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4);


/*
#if(USE_Touch_Panel)
    //show a simple demo for hand-painted tablet, only support for <6inch HD touch e-Paper> at present
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
    TouchPanel_ePaper_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr);
#endif
*/
    //We recommended refresh the panel to white color before storing in the warehouse.
    EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);

    //EPD_IT8951_Standby();
    EPD_IT8951_Sleep();

    //In case RPI is transmitting image in no hold mode, which requires at most 10s
    DEV_Delay_ms(5000);

    DEV_Module_Exit();
    return 0;
}

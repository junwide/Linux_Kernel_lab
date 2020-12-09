/*
  06/01/2016
  Author: Makerbro
  Platforms: ESP8266
  Language: C++
  File: ACROBOTIC_SSD1306.h
  ------------------------------------------------------------------------
  Description: 
  SSD1306 OLED Driver Library.
  ------------------------------------------------------------------------
  Please consider buying products from ACROBOTIC to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use. 
  https://acrobotic.com/
  ------------------------------------------------------------------------
  License:
  Released under the MIT license. Please check LICENSE.txt for more
  information.  All text above must be included in any redistribution. 
*/

#ifndef OLED_H
#define OLED_H

#define pgm_read_byte(addr) (*(unsigned char *)(addr))
#define OLEDFONT(name) static const uint8_t name[]

 #include <linux/types.h>
 #include <linux/i2c.h>
// #include <linux/string.h>
// #include <fcntl.h>
// #include <linux/i2c-dev.h>
// #include <errno.h>
#include "font8x8.h"
#include "font5x7.h"
#include "font6x8.h"
#include "font16x8.h"

#define OLED_WIDTH    128
#define OLED_HEIGHT  64
//screen
#define OLED_H_Max_X                 127    //128 Pixels
#define OLED_H_Max_Y                 63     //64  Pixels
//mode
#define PAGE_MODE                     01           
#define HORIZONTAL_MODE               02
//OLED information
#define OLED_H_Address               0x3C
#define Command_Mode                 0x00       //datasheel
#define Data_Mode                    0x40       //datasheel
//Start Line address
#define Start_Line_addr              0x40      // map to 0 (0~63)
//contrast control 
#define contrast_ctrl                0x81
#define contrast_valu                0xff
//Set Segment Re-map 
#define Segment_Reset                0xA0   //column address 0 is mapped to SEG0 (RESET)
#define Segment_Map                  0xA1   //column address 127 is mapped to
//Memory Mode
#define Set_Memory_Mode              0x20      // as header
#define Page_addressing              0x02
#define Horizontal_mode              0x00
#define Vertical_mode                0x01       // Not Recommand
//entiry Display
#define Follow_RAM                   0xA4   //Output follows RAM content
#define Ignore_RAM                   0xA5   //Output ignores RAM content
//Scan Direction
#define row_scan                     0xC0  
#define com_scan                     0xC8
//display way
#define Normal_Display               0xA6       // normal
#define inverse_Display               0xA7       // inverse 
//multiplex
#define Set_multiplex                0xA8
#define multiplex_reset              0x3f       // (0e-3f)
//display offset
#define Set_Display_offset           0xD0
#define offset_reset                 0x00
//display clock divide ratio/oscillator frequency
#define Set_R_O_freq                 0xD5
#define R_O_reset                    0x80      //[7:4] o      [3:0] CLK
//pre-charge period
#define Set_precharge_p              0xD5
#define precharge_p                  0xf1      //[7:4] charge    [3:0] discharge   22h
//Hardware
#define Set_Hardware                 0xDA
#define hardware_reset               0x12
//Volt
#define Set_V_COMH                   0xDB
#define v_COMH_default               0x40
//enable charge Pump
#define Set_Charge_pump              0x8D
//CMD marco
#define Display_Off                  0xAE       // down
#define Display_On                   0xAF       // up

#define OLED_H_Activate_Scroll_Cmd   0x2F       // Srolling
#define OLED_H_Dectivate_Scroll_Cmd  0x2E       // off Srolling
#define OLED_H_Set_Brightness_Cmd    0x81       // Brightness
//Srolling direction
#define Scroll_Left                   0x00
#define Scroll_Right                  0x01
//Srolling Num
#define Scroll_2Frames                0x7
#define Scroll_3Frames                0x4
#define Scroll_4Frames                0x5
#define Scroll_5Frames                0x0
#define Scroll_25Frames               0x6
#define Scroll_64Frames               0x1
#define Scroll_128Frames              0x2
#define Scroll_256Frames              0x3

///////////////////////////////////////////
// modify this for platform-compatibility
//  I2C device args
///////////////////////////////////////////

//Linux  check 
#define OLED_H_I2C_DEVFILE "/dev/i2c-5" // check "ls /dev/i2c*" and doc
#define OLED_H_I2C_ADDR 0x3c // "i2cdetect -y x"




// static uint8_t m_col;              // Cursor column.
// static uint8_t m_row;              // Cursor row (RAM). 
// static char addressingMode;



//函数
/*
1. 使用字符串前需要设置字体大小
2. 输出字符串前需要清屏
3. 输出时需要调用刷新


*/
int OLED_Probe(struct i2c_client *client, const struct i2c_device_id *id);
int OLED_Remove(struct i2c_client *client);
void OLED_setFont(const uint8_t* font);

//void OLED_init(struct i2c_client *client);

// void OLED_fill_screen(
//     uint8_t chXpos1, 
//     uint8_t chYpos1, 
//     uint8_t chXpos2, 
//     uint8_t chYpos2, 
//     uint8_t chDot);
// core
void OLED_write_byte(uint8_t Data,uint8_t Mode);
int OLED_putString(
    uint8_t chXpos, 
    int8_t chYpos,
    const char *string,
    uint8_t chMode);
int OLED_addString(
    const char *string,
    uint8_t chMode
);
int OLED_putChar(
    uint8_t chXpos, 
    uint8_t chYpos, 
    uint8_t chChr, 
    uint8_t chMode);
int divide_Cmd(const char *string);
int OLED_draw_point(uint8_t chXpos, uint8_t chYpos, uint8_t chPoint);
void OLED_refresh_gram(void);
void OLED_clear_screen(uint8_t chFill);
int OLED_dram_line(const char *string);
int OLED_draw_circle(const char *string);
int abs_usr(int x);
#endif

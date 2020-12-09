/*
    MIT License

    Copyright (c) 2018, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int get_args_list(char *str, char* argv[])
{
    int argc = 0;
    *argv = str;
    argv++;
    while (*str)
    {
        if (*str == '\n') *str = '\0';
        if (*str == '\r') *str = '\0';
        if ((*str == ' ') || (*str == ','))
        {
            do
            {
                *str = '\0';
                str++;
            }
            while (*str == ' ');
            if (*str)
            {
                argc++;
                *argv = str;
                argv++;
            }
        }
        str++;
    }
    argc++;
    return argc;
}

int atoi_(char *str)
{
    return strtol(str, NULL, 10);
}

int atoi_h(char *str)
{
    return strtol(str, NULL, 16);
}

int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

uint8_t *atoi_b(char *str)
{
    static uint8_t buffer[4096];
    uint8_t *ptr = buffer;
    while (*str)
    {
        uint8_t data = hex2int(*str) << 4;
        str++;
        if (*str == '\0') break;
        data |= hex2int(*str);
        *ptr = data;
        ptr++;
        str++;
    }
    return buffer;
}


int execute_mono_cmd(int argc, char *argv[])
{
    if (!strcmp(argv[0], "quit")) return -1;
    else if (!strcmp(argv[0], "clear"))
        argc > 1 ? ssd1306_fillScreen(atoi_h(argv[1])) : ssd1306_clearScreen();
    else if (!strcmp(argv[0], "rect"))
        ssd1306_drawRect(atoi_(argv[1]), atoi_(argv[2]), atoi_(argv[3]), atoi_(argv[4]));
    else if (!strcmp(argv[0], "line"))
        ssd1306_drawLine(atoi_(argv[1]), atoi_(argv[2]), atoi_(argv[3]), atoi_(argv[4]));
    else if (!strcmp(argv[0], "bitmap"))
        gfx_drawMonoBitmap(atoi_(argv[1]), atoi_(argv[2]), atoi_(argv[3]), atoi_(argv[4]), atoi_b(argv[5]));
    else if (argv[0][0] != '\0')
        fprintf(stderr, "commands:\nclear [pattern]\nrect x1,y1,x2,y2\n"
                        "line x1,y1,x2,y2\n"
                        "bitmap x1,y1,width,height,bitmap_hex\n");
    return 0;
}

int main(int argc, char *argv[])
{
    fprintf(stderr, "Enter command\n");
    char str[16384];
    while (fgets(str, sizeof str, stdin))
    {
        char* arg_list[128];
        if (execute_mono_cmd(get_args_list(str, &arg_list[0]), arg_list) < 0)
        {
            break;
        }
    }
    ssd1306_intf.close();
    return 0;
}


#include "io.h"
#include "string.h"
#include "serial.h"
#include "interrupts.h"
#include "memory_segments.h"


/**
 * For the framebuffer,
 *
 * example 0x4128
 * 41: ascii of character
 * 2: foreground color
 * 8: background color
 * hence it is defined as follows.
 *
 * @TODO split them into separate header file.
 */

/* colors */
#define FB_BLACK 0
#define FB_BLUE 1
#define FB_GREEN 2
#define FB_CYAN 3
#define FB_RED 4
#define FB_MAGENTA 5
#define FB_BROWN 6
#define FB_LIGHT_GREY 7
#define FB_DARK_GREY 8
#define FB_LIGHT_BLUE 9
#define FB_LIGHT_GREEN 10
#define FB_LIGHT_CYAN 11
#define FB_LIGHT_RED 12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN 14
#define FB_WHITE 15

/* framebuffer mm io */
char* fb = (char *) 0x000B8000;
char* dataMarkStart = (char*) 0x00200000;
char* dataMarkEnd = (char*) 0x010FF7FE;
char* dataRealStart = (char*) 0x010FF7FF;
char* dataRealEnd = (char*) 0x01FFEFFE;

/**
 * display character c on the position i with color fg and bg.
 *
 * @param i the position, 0 for the first line, first col. 16 for the second line.
 * @param c the displayed character
 * @param fg foreground color
 * @param bg background color
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4 | (bg & 0x0F));
}

/* io ports for cursors */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT 0x3D5

#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND 15

/**
 * move the cursor to the given position
 *
 * @param short pos: 16 bit totally
 *      - first 8 bit: the row
 *      - last 8 bit: the colomn
 */
void fb_move_cursor(unsigned short pos) 
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT, pos & 0x00FF);
}

void fb_write_string(int offset, char* s, int length)
{
    int i;
    for(i = 0; i < length; i = i + 1)
    {
        fb_write_cell(offset + i*2, s[i], FB_BLACK, FB_WHITE);
    }
}

void fb_clear()
{
    for(int i = 0; i < 80*25; i++)
    {
        fb_write_cell(i*2, ' ', FB_BLACK, FB_BLACK);
    }
}

int isMemAvailable(char* baseAddress, int size)
{
    for(char* c = baseAddress; c < (baseAddress+size); c++)
    {
        if(*c == 1)
        {
            return 0; //this base address won't work out
        }
    }
    return 1; //this base address will work
}

void markMemory(char* baseAddress, int size)
{
    for(char* c = baseAddress; c < baseAddress + size; c++)
    {
        *c = 1;
    }
}

char* getMem(int numBytes)
{
    //How much do I have to add to my found memory in first half to get 
    //corresponding memory in second half.
   int dataMapOffset = (int)dataRealStart - (int)dataMarkStart;
   for(char* c = dataMarkStart; c <= dataMarkEnd; c++)
   {
       if(*c == 0)
       {
           if(isMemAvailable(c, numBytes))
           {
               markMemory(c, numBytes); //mark the memory as not available
               return c + dataMapOffset;
           }
       }
   }
   return 0x0; //didn't find available memory
}

void freeMem(char* baseAddress, int numBytes)
{
    for(int i = 0; i < numBytes; i++)
    {
        *(baseAddress + i) = 0;
    }
}

void unmarkAllMemory()
{
    for(char* c = dataMarkStart; c <= dataMarkEnd; c++)
    {
        *c = 0;
    }
}

int main() 
{
    //prepare OS for handing out mem
    unmarkAllMemory();

//    char* s;
    fb_clear();

	
	test_serial_port();
    segments_install_gdt();
    interrupts_install_idt();
    return 0;
}
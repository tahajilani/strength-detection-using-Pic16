/* 
 * File:   project.c
 * Author: taha
 *
 * Created on August 1, 2020, 6:02 AM
 */

#include <xc.h>
#include <stdlib.h>
#include <pic16f877a.h>

#define _XTAL_FREQ 3276800 
void LCD_write(unsigned char c, unsigned char rs);
void LCD_clear(void);                              // function to clear LCD display and home the cursor
/* LCD_initialise: Send initialisation commands to LCD. You MUST run this 
 * function once before prior to using any of the LCD display functions
 * contained in this library. */
void LCD_initialise(void);                             

/* LCD_clear: Clears the display and homes the cursor to position (0,0) */
void LCD_clear(void);              

/* LCD_putch: Write a single character to the LCD display at the current cursor
 * position  */
void LCD_putch(unsigned char);                     

/* LCD_puts: Write string s to LCD. NB for const string use the LCD_putsc() 
 * function */
void LCD_puts(unsigned char *);                        

/* LCD_putsc: Write const string s to LCD. Use this function to display strings
 * that you have declared as const so they are stored in program memory */
void LCD_putsc(const unsigned char *);                 

/* LCD_cursor: Move cursor to position (row , column). row may assume a value 
 * of 0 or 1 only representing the top and bottom line respectively. the column 
 * parameter may assume any value between 0 (leftmost) and 15 (rightmost) */

void LCD_cursor (unsigned char, unsigned char);   

/* LCD_cursor: Displays a numerical value on the LCD display. The function will
 * calculate and display the correct number of digits up to a maximum of 4. Thus
 * it will correctly display values ranging from 0 to 9999. */

/* LCD_display_value: Displays a numerical value on the LCD display. The 
 * function will calculate and display the correct number of digits for signed 
 * or unsigned values. */

void LCD_display_value (long value);    

/* LCD_display_float: Displays a floating point numerical value on the LCD 
 * This function take the parameter 'value' and displays it to the number of
 * decimal places specified by the parameter 'dplaces'. NOTE: The function
 * is limited to displaying 4 significant figures due to headroom limitations
 * of the 24 bit float type in MPLAB X. Attempting to display more than 4
 * significant figures will lead to the string "ERR" being displayed. */

                   

// LCD Driver module
// LCD module is connected as follows:
// RB0 - RB3 -> DB4 - DB7   (data lines)
// RB4 -> RS                (control line)
// RB5 -> E                 (control line)

// Define some useful constants
#define RS RB4               // RS control line is bit 4 of port B
#define E RB5                // E control line is bit 5 of port B

#define LCD_INIT 0x33             // command to initialise LCD display
#define FOUR_BIT_MODE 0x32        // command to make the LCD transfer instructions and data in 4 bit chunks rather than 8
#define TWO_LINE_MODE 0x2c        // command to put the LCD in two-line mode
#define MOVE_AFTER_WRITE 0x06     // command to increment cursor position automatically
#define DISPLAY_INIT 0x0c         // command to turn display on, switch off the cursor and stop it blinking

#define WRITE_DELAY 200                // constant used to set write delay time (50 us required)
#define BIT_DELAY 2                    // constant used to set pulse duration (2/3 us required)
#define INST_DELAY 1000                // constant used to det instruction delay (1.53 ms required)

/*
 * 
 */
void main(void) {
    char myString[] = {"Strength"};
    TRISDbits.TRISD0 = 1; //attach PIR sensor placed at entrance
    TRISDbits.TRISD1 = 1;//attach PIR sensor placed at exit
    unsigned short strength=0;
    LCD_initialise();
    LCD_puts(myString);
    while (1) {
        if (PORTDbits.RD0 == 1) {
            strength++;
        }
        if (PORTDbits.RD1 == 1) {
            strength--;
        }
        LCD_cursor(4, 1);
        LCD_display_value(strength);
        __delay_ms(1000); //1 sec delay between reads 
    }


    
   

}
// Function to initialise LCD module



void LCD_initialise(void)
{
    unsigned int counter;
        // INITIALISE LCD
        LCD_write(LCD_INIT, 0);             // command to initialise LCD display, (RS = 0)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction
        
        // Set LCD for 4-bit mode
        LCD_write(FOUR_BIT_MODE, 0);        // command to set LCD in 4-bit mode, (RS = 0)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction

        // set LCD in up for two lines
        LCD_write(TWO_LINE_MODE, 0);        // command to set LCD for two lines, (RS = 0)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction

        // Select move after write
        LCD_write(MOVE_AFTER_WRITE, 0);     // command to set LCD to increment cursor pos. automatically (RS = 0)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction

        // Configure display & cursor
        LCD_write(DISPLAY_INIT, 0);         // command to configure display, (RS = 0)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction

        LCD_clear();
}



// LCD_write() function is a basic write subroutine that could be an instruction or data
//  Note: PORTB interrupts are not available during execution of this function
void LCD_write(unsigned char c, unsigned char rs)
{
        unsigned char temp, intsON;     // temporary variable to hold ms 4 bits of c
        unsigned char RSmask = 0;       // used to set or reset bit 4 of PORTB
        unsigned char TRISB_BAK;        // Current status of port B data direction register
        unsigned char INTCON_BAK;
        unsigned int counter;            // Used for producing required delay when pulsing line E
        //unsigned char PORTB_BAK;

        // INITIALISATION
        intsON = INTCONbits.GIE;        // Store GIE status and disable interrupts
        INTCONbits.GIE = 0;
        TRISB_BAK = TRISB;              // Get current state of TRISB register
        TRISB = 0x00;                   // Set PORTB bits 0 - 7 as outputs

        if (rs == 1) RSmask = 0x10;        // set up bit mask for RS line

        // set MS 4 bits of c up on RB0 - RB3
        temp = c;
        PORTB = (temp >> 4) | RSmask;     // shift MS 4 bits into LS 4 bits, set/rst RS and present on PORTB

        // pulse E high then low
        // set E high and wait a bit
        E = 1;                                // set E line high
        for (counter = 0; counter < BIT_DELAY; counter++); // pulse delay for E
        // set E low and wait a bit
        E = 0;                                // set E line low
        for (counter = 0; counter < BIT_DELAY; counter++); // pulse delay for E
        
        // set LS 4 bits up on RB0 - RB3
        PORTB = (c & 0x0F) | RSmask;      // present LS 4 bits on RB0 - RB3 and set/rst RS line

        // pulse E high then low
        // set E high and wait a bit
        E = 1;                                // set E line high
        for (counter = 0; counter < BIT_DELAY; counter++); // pulse delay for E
        // set E low and wait a bit
        E = 0;                                // set E line low
        for (counter = 0; counter < BIT_DELAY; counter++); // pulse delay for E
        
        PORTB = 0x10;                     // set E high again  to avoid annoying LED flicker

        TRISB = TRISB_BAK;                // Restore state of TRISB register
        temp = PORTB;                     // Dummy read of PORTB to avoid retriggering RBIF
        
        INTCONbits.RBIF=0;                // Clear RBIF interrupt flag to avoid false trigger on exit
        INTCONbits.INTF=0;                // Clear RBIF interrupt flag to avoid false trigger on exit
        
        if(intsON) GIE = 1;
}



//putch(c) function is character output routine
void LCD_putch(unsigned char c)
{
        unsigned int counter;
        LCD_write(c, 1);                // call basic LCD write routine with RS high (send data)
        for (counter = 0; counter < WRITE_DELAY; counter++); //wait until LCD has completed write operation
}

//LCD_clear() function is an instruction to clear all data from LCD display and return cursor to beginning of display
void LCD_clear(void)
{
        unsigned int counter;
        LCD_write(1, 0);                // call basic write function with instruction 1, (clear display) and RS 0, (write instruction)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction
        LCD_write(2, 0);                // call basic write function with instruction 2, (cursor home) and RS = 0, (write instruction)
        for (counter = 0; counter < INST_DELAY; counter++); //wait until LCD has completed instruction
}

// Function to write null terminated string to LCD display
void LCD_puts(unsigned char *s)
{
        unsigned char c = 255;
        unsigned char i = 0;
        while ((c > 0) && (i < 16)) //16 is the maximum length of the string
        {
          c = s[i];
          if (c != 0) LCD_putch(c);
          i++;
        }
}

// Function to write null terminated string to LCD display
void LCD_putsc(const unsigned char *s)
{
        unsigned char c = 255;
        unsigned char i = 0;
        while ((c > 0) && (i < 16)) //16 is the maximum length of the string
        {
          c = s[i];
          if (c != 0) LCD_putch(c);
          i++;
        }
}

void LCD_cursor (unsigned char x, unsigned char y )
{
    if ( y==0 )
    {
        /* position for line 0     */
        y=0x80 ;
    }
    else
    {
        /* position for line 1     */
        y=0xc0 ;
    }

    LCD_write( y+x, 0) ;
}


void LCD_display_value (long value)
{
    char buffer[12];          // Buffer for storing ltoa() conversions)
                              // Max 10 digits + null and sign
    ltoa(buffer, value, 10);  // Do conversion using stdlib function itoa()
    LCD_puts(buffer);         // Output to LCD display
}



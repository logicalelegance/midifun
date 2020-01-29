/*
 * display.c
 *
 *  Created on: Jan 23, 2020
 *      Author: Chris White
 */


#include "stm32f3xx_hal.h"

// PIN Definitions for GPIOs
#define CS_PIN GPIO_PIN_9
#define CMD_DATA_PIN GPIO_PIN_8
#define RST_PIN GPIO_PIN_12

// ST7735 SPI commands
#define ST7735_TFTWIDTH_128 128  // for 1.44 and mini
#define ST7735_TFTWIDTH_80 80    // for mini
#define ST7735_TFTHEIGHT_128 128 // for 1.44" display
#define ST7735_TFTHEIGHT_160 160 // for 1.8" and mini display

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define	ST7735_BLACK   0x0000

#define ST7735_XSTART 0
#define ST7735_YSTART 0


extern SPI_HandleTypeDef hspi1;

static const uint8_t
ST7735R_Init1[] =  {                       // 7735R init, part 1 (red or green tab)
		15,                             // 15 commands in list:
		ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
		150,                          //     150 ms delay
		ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
		255,                          //     500 ms delay
		ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
		0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
		0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
		0x01, 0x2C, 0x2D,             //     Dot inversion mode
		0x01, 0x2C, 0x2D,             //     Line inversion mode
		ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
		0x07,                         //     No inversion
		ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
		0xA2,
		0x02,                         //     -4.6V
		0x84,                         //     AUTO mode
		ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
		0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
		ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
		0x0A,                         //     Opamp current small
		0x00,                         //     Boost frequency
		ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
		0x8A,                         //     BCLK/2,
		0x2A,                         //     opamp current small & medium low
		ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
		0x8A, 0xEE,
		ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
		0x0E,
		ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
		ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
		0xC8,                         //     row/col addr, bottom-top refresh
		ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
		0x05 },
ST7735R_Init2[] = {
		2,           //  2 commands in list:
		ST77XX_CASET, 4,              //  1: Column addr set, 4 args, no delay:
		0x00, 0x02,                   //     XSTART = 0
		0x00, 0x7F + 0x02,              //     XEND = 127
		ST77XX_RASET, 4,              //  2: Row addr set, 4 args, no delay:
		0x00, 0x01,                   //     XSTART = 0
		0x00, 0x9F + 0x01 },            //     XEND = 159
ST7735R_Init3[] = {
		4,                           //  4 commands in list:
		ST7735_GMCTRP1, 16, //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
		0x02, 0x1c, 0x07, 0x12, //     (Not entirely necessary, but provides
		0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
		0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
		ST7735_GMCTRN1, 16, //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
		0x03, 0x1d, 0x07, 0x06, //     (Not entirely necessary, but provides
		0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
		0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
		ST77XX_NORON, ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
		10,                           //     10 ms delay
		ST77XX_DISPON, ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
		100 };                        //     100 ms delay


void ST7735_Select(void)
{
	HAL_GPIO_WritePin(GPIOB, CS_PIN, GPIO_PIN_RESET);
}

void ST7735_Deselect(void)
{
	HAL_GPIO_WritePin(GPIOB, CS_PIN, GPIO_PIN_SET);
}


void ST7735_Cmd_Write(const uint8_t cmd, const uint8_t *args, uint8_t num_args)
{


	// Set command mode
	HAL_GPIO_WritePin(GPIOB, CMD_DATA_PIN, GPIO_PIN_RESET);

	// Send command
	HAL_SPI_Transmit(&hspi1, &cmd, 1, 25);

	// Set data mode
	if ((args != NULL) && (num_args > 0)) {
		HAL_GPIO_WritePin(GPIOB, CMD_DATA_PIN, GPIO_PIN_SET);
		HAL_SPI_Transmit(&hspi1, args, num_args, 25);
	}
}

void ST7735_Cmd_List_Send(const uint8_t *cmd_list) {
	uint8_t num_commands, cmd, num_args, cmd_index = 0;

	num_commands = cmd_list[cmd_index];
	cmd_index++;

	while (num_commands--) {
		uint16_t delay_ms;
		cmd = cmd_list[cmd_index];
		cmd_index++;

		num_args = cmd_list[cmd_index];
		cmd_index++;

		delay_ms = num_args & ST_CMD_DELAY;
		num_args &= ~ST_CMD_DELAY;
		ST7735_Cmd_Write(cmd, &cmd_list[cmd_index], num_args);

		if (num_args) {
			cmd_index += num_args;
		}

		if (delay_ms) {
			delay_ms = cmd_list[cmd_index];
			cmd_index++;
			if (delay_ms == 255) {
				delay_ms = 500;
			}
			HAL_Delay(delay_ms);
		}
	}
}

static void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    // column address set
    uint8_t data[] = { 0x00, x0 + ST7735_XSTART, 0x00, x1 + ST7735_XSTART };
    ST7735_Cmd_Write(ST77XX_CASET, data, 4);

    // row address set
    data[1] = y0 + ST7735_YSTART;
    data[3] = y1 + ST7735_YSTART;
    ST7735_Cmd_Write(ST77XX_RASET, data, 4);

    // write to RAM
    ST7735_Cmd_Write(ST77XX_RAMWR, NULL, 0);
}

void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // clipping
    if((x >= ST7735_TFTWIDTH_128) || (y >= ST7735_TFTHEIGHT_160)) return;
    if((x + w - 1) >= ST7735_TFTWIDTH_128) w = ST7735_TFTWIDTH_128 - x;
    if((y + h - 1) >= ST7735_TFTHEIGHT_160) h = ST7735_TFTHEIGHT_160 - y;

    ST7735_Select();
    ST7735_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[] = { color >> 8, color & 0xFF };
	HAL_GPIO_WritePin(GPIOB, CMD_DATA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, CS_PIN, GPIO_PIN_RESET);

    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            HAL_SPI_Transmit(&hspi1, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    ST7735_Deselect();
}

void ST7735_FillScreen(uint16_t color) {
    ST7735_FillRectangle(0, 0, ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, color);
}

void test1(void)
{

	// Select
	ST7735_Select();

	// Reset
	HAL_GPIO_WritePin(GPIOF, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOF, RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOF, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(200);

	HAL_Delay(1);

	ST7735_Cmd_List_Send(ST7735R_Init1);
	ST7735_Cmd_List_Send(ST7735R_Init2);
	ST7735_Cmd_List_Send(ST7735R_Init3);

    ST7735_FillScreen(ST7735_BLACK);
    ST7735_Deselect();
}

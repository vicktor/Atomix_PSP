/*
Atomix PSP Remake
Copyright (C) 2006-03 naboo

Homepage: http://www.postumum.com
E-mail:   postumum@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <psptypes.h>
#include <psppower.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "graphics.h"
#include "csprite.h"
#include "funciones.h"

#define RGB(r, g, b) ((b << 16) | (g << 8) | r)	
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define MAP_WIDTH 18
#define MAP_HEIGHT 13
#define TILE_WIDTH  16
#define TILE_HEIGHT 16
#define OFFSET_MAP 216
// GLOBAL VARS-----------------------------------------------------------------
typedef struct {
   int tile;
} MAP_INFO;

struct Puntuaciones {
	int puntos;
	char player[3];
} puntuaciones_max[10];

struct Molecule {
		int x, y;
		int tipo;
		int link;
		int dlink;
		int activo;
		int seleccionado;
		int ID;
} molecule[16];

struct Solve {
		int x, y;
		int tipo;
		int link;
		int dlink;
		int activo;
} solve[16];

MAP_INFO map[MAP_WIDTH][MAP_HEIGHT];
char *passw[] = {"EGG", "VIK", "VGA", "BAD", "DUO", "NOP", "MAR", "MAC", "KEY", "ZOO", "SKA", "PUT", "PAD", "ICE", "HOT", "FUN", "EYE", "CAT", "JAN" };

struct timeval hora;

Image* img_main;
Image* img_bg;
Image* img_tiles;
Image* img_marcador;
Image* img_fuente;
Image* img_info;
Image* img_atoms;
Image* img_links;

int mapx, mapy, level;
int font_table[32];
int game_done;
int option_selected;
int molecule_selected, molecules;
int movimientos, fondo;
int editando;
SceCtrlData PSP_Pad;
u32 PSP_Pad_last, PSP_Pad_new;

// PSP DEFAULT DECLARATIONS----------------------------------------------------

PSP_MODULE_INFO("Atomix for PSP. Naboo", 0, 1, 1);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
  int thid = 0;

  thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
  if(thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
  }
  return thid;
} 

// GAME FUNCTIONS--------------------------------------------------------------

void SaveLevel() {
	int x,y;
	FILE *outfile;
	char buffer[30];
	sprintf(buffer, "map%02i.lev", level);

 	if((outfile = fopen(buffer, "wt")) != NULL) { 		
 		fputc(mapx, outfile);
 		fputc(mapy, outfile);
 		for (x=0;x<MAP_WIDTH;x++) 
 			for (y=0;y<MAP_HEIGHT;y++) 
 				fputc(map[x][y].tile,outfile);
 	}
 	fclose(outfile);

	sprintf(buffer, "mol%02i.lev", level);

 	if((outfile = fopen(buffer, "wt")) != NULL) { 		
 		fputc( molecules, outfile);
 		for (x=0;x<molecules;x++)  {
 			fputc(molecule[x].x, outfile);
 			fputc(molecule[x].y, outfile);
 			fputc(solve[x].x, outfile);
 			fputc(solve[x].y, outfile);
 			fputc(molecule[x].tipo, outfile);
 			fputc(molecule[x].link, outfile);
 			fputc(molecule[x].dlink, outfile);
 			fputc(molecule[x].ID, outfile);
 		}
 	}
 	fclose(outfile);
 	
}

void LoadLevel() {
	int x,y;
	FILE *infile;
	char buffer[30];
	
	sprintf(buffer, "map%02i.lev", level);

 	if((infile = fopen(buffer, "r")) != NULL) { 		
 		mapx = fgetc(infile);
 		mapy = fgetc(infile);
 		for (x=0;x<MAP_WIDTH;x++) 
 			for (y=0;y<MAP_HEIGHT;y++) 
 				map[x][y].tile = fgetc(infile);
 	} else {
 		for (x=0;x<MAP_WIDTH;x++) 
 			for (y=0;y<MAP_HEIGHT;y++) {
 				map[x][y].tile = 6;
 				if (x == 0 || x == MAP_WIDTH-1) map[x][y].tile = 1;
 				if (y == 0 || y == MAP_HEIGHT-1) map[x][y].tile = 1;
 			}
 		
 	}
 	fclose(infile);
 	
	for (x=0;x<16;x++)  {
		molecule[x].activo = 0;
		solve[x].activo = 0;
	}

 	x = 0;
	sprintf(buffer, "mol%02i.lev", level);
 	if((infile = fopen(buffer, "r")) != NULL) { 		
 		molecules = fgetc(infile);
 		if (molecules > 16) molecules = 16;
 		
 		for (x=0;x<molecules;x++)  {
 			molecule[x].x = fgetc(infile);
 			molecule[x].y = fgetc(infile);
 			solve[x].x = fgetc(infile);
 			solve[x].y = fgetc(infile);
 			molecule[x].tipo = fgetc(infile);
 			solve[x].tipo = molecule[x].tipo;
 			molecule[x].link = fgetc(infile);
 			solve[x].link = molecule[x].link;
 			molecule[x].dlink = fgetc(infile);
 			solve[x].dlink = molecule[x].dlink;
 			molecule[x].ID = fgetc(infile);
 			molecule[x].activo = 1;
 			solve[x].activo = 1;
 		}
 	}
 	fclose(infile);
	movimientos = 0;
	
	if (fondo > 3) fondo = 0;
	sprintf(buffer, "bg%i.png", fondo);
	if (img_bg != NULL) freeImage(img_bg);
	img_bg = loadImage(buffer);
	fondo++;
}

void writeFont(int x, int y, const char* text) {
	int y_img, font;
	for (int c = 0; c < strlen(text); c++) {
		char ch = text[c];
		font = (int)ch;
		y_img = 0;
		if (font > 95) {
			y_img = 16;
			font -= 64;
		} else {
			if (font > 63) {
				y_img = 16;
				font -= 32;
			}
		}
		font -= 32;
			
		blitAlphaImageToScreen(font_table[font] ,y_img ,16, 16, img_fuente, x, y);
		x +=16;
	}	
}

void ShowMolecule() {
	int i;
	char *level_name[] = {"WATER", "METHANE", "METHANOL", "ETHYLENE", "PROPYLEN","ETHANAL","ETHANOL","ACETICACID","DIMETHYLETHER","FORMALDEHYD","ACETONE","TRANSBUTYLEN","BUTHANOL","PROPANAL","PROPANAL","PYRAN","CYCLO-BUTHANE","ETHAN","LACTIC-ACID","GLYCERIN"};

	writeFont(170,10,level_name[level-1]);
  for (i=0;i<molecules;i++) {
  	if (solve[i].activo == 1) {
			blitAlphaImageToScreen((molecule[i].tipo-1)*16 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_atoms, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);
			if (molecule[i].dlink == 0) {
				if (molecule[i].link & 1) blitAlphaImageToScreen(0 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);
				if (molecule[i].link & 4) blitAlphaImageToScreen(32 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
				if (molecule[i].link & 16) blitAlphaImageToScreen(64 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
				if (molecule[i].link & 64) blitAlphaImageToScreen(96 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
			} else {
				if (molecule[i].dlink & 1) blitAlphaImageToScreen(128 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);
				if (molecule[i].dlink & 4) blitAlphaImageToScreen(144 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
				if (molecule[i].dlink & 16) blitAlphaImageToScreen(160 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
				if (molecule[i].dlink & 64) blitAlphaImageToScreen(176 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
			}
			if (molecule[i].link & 2) blitAlphaImageToScreen(16 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
			if (molecule[i].link & 32) blitAlphaImageToScreen(80 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
			if (molecule[i].link & 8) blitAlphaImageToScreen(48 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);	
			if (molecule[i].link & 128) blitAlphaImageToScreen(112 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, 50+solve[i].x*TILE_WIDTH, 150+solve[i].y*TILE_HEIGHT);										
 		}
 	}
}

void ShowCredits() {
	int done=0;
	char texto[80];
	while (!done) {
		blitImageToScreen(0 ,0 ,480, 272, img_info, 0, 0);
		sprintf(texto,"ATOMIX PSP REMAKE 1.0");
		writeFont(52,60,texto);
		sprintf(texto,"CODED BY NABOO");
		writeFont(52,100,texto);

		sprintf(texto,"HTTP://WWW.POSTUMUM.COM");
		writeFont(52,130,texto);
		sprintf(texto,"LEVELS: Jens Finke");
		writeFont(52,180,texto);
		sprintf(texto,"PRESS X");
		writeFont(328,226,texto);

		sceCtrlReadBufferPositive(&PSP_Pad, 1);
  	PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  	PSP_Pad_last = PSP_Pad.Buttons;

  	if (PSP_Pad_new != 0) {
			if (PSP_Pad_new & PSP_CTRL_CROSS) done = 1;
		}
		sceDisplayWaitVblankStart(); 
		flipScreen(); 
	}
}

void GoLevel() {
	int done=0,letra=0, i;
	char pass[3];
	strncpy(pass,"AAA",3);
	pass[3] = 0x00;
	while (!done) {
		blitImageToScreen(0 ,0 ,480, 272, img_info, 0, 0);
		writeFont(116,100,"ENTER LEVEL CODE");
		writeFont(218,140,pass);
		writeFont(218+letra*16,156,"-");
		sceCtrlReadBufferPositive(&PSP_Pad, 1);
  	PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  	PSP_Pad_last = PSP_Pad.Buttons;

  	if (PSP_Pad_new != 0) {
			if (PSP_Pad_new & PSP_CTRL_CROSS) done = 1;
			if (PSP_Pad_new & PSP_CTRL_RIGHT) if (letra < 2) letra++;
			if (PSP_Pad_new & PSP_CTRL_LEFT) if (letra > 0) letra--;
			if (PSP_Pad_new & PSP_CTRL_UP) if (pass[letra] < 90) pass[letra]++;
			if (PSP_Pad_new & PSP_CTRL_DOWN) if (pass[letra] > 65) pass[letra]--;
		}
		sceDisplayWaitVblankStart(); 
		flipScreen(); 
	}
	level = 0;
	for (i=0;i<19;i++) {
		if (strcmp(pass,passw[i]) == 0) {
			level = i;
			break;
		}
	}
	if (level != 0) option_selected = 1;
}

void MainMenu() {
	int done = 0, y=70;
	
	while (!done) {
		blitImageToScreen(0 ,0 ,480, 272, img_info, 0, 0);
	  writeFont(184,70,"Start!");
	  writeFont(168,105,"Go Level");
	  writeFont(176,140,"Credits");
	  writeFont(184,175,"Editor");
	  writeFont(128,y,">           <");
	  fondo = 0;
		sceCtrlReadBufferPositive(&PSP_Pad, 1);
  	PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  	PSP_Pad_last = PSP_Pad.Buttons;

  	if (PSP_Pad_new != 0) {
			if (PSP_Pad_new & PSP_CTRL_CROSS) done = 1;
			if (PSP_Pad_new & PSP_CTRL_UP) y -= 35;
			if (PSP_Pad_new & PSP_CTRL_DOWN) y += 35;		
		}
		if (y < 70) y = 175;
		if (y > 175) y = 70;	
		sceDisplayWaitVblankStart(); 
		flipScreen(); 
	}
	switch ( y )
    {
      case 70:
      	level = 1;
        option_selected = 1;
        break;	
      case 105:
        option_selected = 2;
        break;	
      case 140:
        option_selected = 3;
        break;	
      case 175:
        option_selected = 4;
        break;	
      case 210:
        option_selected = 5;
        break;	
    }
}

void LoadGFX() {
  img_tiles = loadImage("tiles.png");
  img_marcador = loadImage("marcador.png");
  img_fuente = loadImage("font.png");
  img_info = loadImage("info.png");
  
  img_atoms = loadImage("atoms.png");
  img_links = loadImage("links.png");
  
  for (int i=0; i<32; i++)
  	font_table[i] = i*16;
}

int CheckAtom(int x_pos, int y_pos) {
	int i, atomo=0;
	for (i=0;i<molecules;i++)  {
		if (molecule[i].activo == 1) {
			if (molecule[i].x == x_pos && molecule[i].y == y_pos) {
				atomo = molecule[i].ID;
			}
		}
	}
	return atomo;
}

void Editor_DrawLevel() {
	int x,y, tile;
	tile = (level % 5)*16;
	
	for (x=0;x<MAP_WIDTH;x++) {
		for (y=0;y<MAP_HEIGHT;y++) {
			if (map[x][y].tile == 101 ) {
				blitImageToScreen(80 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_tiles, OFFSET_MAP+x*TILE_WIDTH, 52+y*TILE_HEIGHT);
			}
			if (map[x][y].tile == 102 ) {
				blitImageToScreen(tile ,0 ,TILE_WIDTH, TILE_HEIGHT, img_tiles, OFFSET_MAP+x*TILE_WIDTH, 52+y*TILE_HEIGHT);
			}
		}
	}
	for (x=0;x<molecules;x++)  {
		if (molecule[x].activo == 1) {
			blitAlphaImageToScreen((molecule[x].tipo-1)*16 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_atoms, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);
			if (molecule[x].dlink != 0) {
				if (molecule[x].dlink & 1) blitAlphaImageToScreen(128 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);
				if (molecule[x].dlink & 4) blitAlphaImageToScreen(144 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
				if (molecule[x].dlink & 16) blitAlphaImageToScreen(160 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
				if (molecule[x].dlink & 64) blitAlphaImageToScreen(176 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			}
			if (molecule[x].link & 1) blitAlphaImageToScreen(0 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);
			if (molecule[x].link & 4) blitAlphaImageToScreen(32 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			if (molecule[x].link & 16) blitAlphaImageToScreen(64 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			if (molecule[x].link & 64) blitAlphaImageToScreen(96 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			
			if (molecule[x].link & 2) blitAlphaImageToScreen(16 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			if (molecule[x].link & 32) blitAlphaImageToScreen(80 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			if (molecule[x].link & 8) blitAlphaImageToScreen(48 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
			if (molecule[x].link & 128) blitAlphaImageToScreen(112 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_links, OFFSET_MAP+molecule[x].x*TILE_WIDTH, 52+molecule[x].y*TILE_HEIGHT);	
		}
	}
	ShowMolecule();
}

void Editor_PutCursor() {
	if (molecule_selected == 0) {
		blitAlphaImageToScreen(96 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_tiles, OFFSET_MAP+mapx*16, 52+mapy*16);
	} else {
		blitAlphaImageToScreen(112 ,0 ,TILE_WIDTH, TILE_HEIGHT, img_tiles, OFFSET_MAP+mapx*16, 52+mapy*16);
	}
}

void Editor_DrawScores() {
	char buffer[30];
	sprintf(buffer, "LEVEL: %02i", level);
	writeFont(10,10,buffer);
	if (editando == 0) {
		sprintf(buffer, "MOVEMENTS: %i", movimientos);
		writeFont(10,30,buffer);
	}
	blitAlphaImageToScreen(0 ,0 ,132, 16, img_marcador, 24, 118);
}

void DrawScene() {
	blitImageToScreen(0 ,0 ,480, 272, img_bg, 0, 0);
	
	Editor_DrawScores();
	Editor_DrawLevel();
 	Editor_PutCursor();
	sceDisplayWaitVblankStart(); 
	flipScreen(); 
}

void MoveAtom(int x, int y) {
	int i, delay;
	if (x < 0) {
		i = mapx-1;
		while (map[i][mapy].tile == 101) {
			if (CheckAtom(i,mapy) != 0) break;
			i--;
		}
		i++;
		mapx = i;
		if (molecule[molecule_selected-1].x != i) movimientos++;
		delay = 0;
		while (molecule[molecule_selected-1].x > i) {
			if ((delay % 6) == 0) molecule[molecule_selected-1].x -= 1;
			delay++;
			if (delay == 12) delay = 0;
			DrawScene();
		}
	}
	if (x > 0) {
		i = mapx+1;
		while (map[i][mapy].tile == 101) {
			if (CheckAtom(i,mapy) != 0) break;
			i++;
		}
		i--;
		mapx = i;
		if (molecule[molecule_selected-1].x != i) movimientos++;
		delay = 0;
		while (molecule[molecule_selected-1].x < i) {
			if ((delay % 6) == 0) molecule[molecule_selected-1].x += 1;
			delay++;
			if (delay == 12) delay = 0;
			DrawScene();
		}
	}
	if (y < 0) {
		i = mapy-1;
		while (map[mapx][i].tile == 101) {
			if (CheckAtom(mapx,i) != 0) break;
			i--;
		}
		i++;
		mapy = i;
		if (molecule[molecule_selected-1].y != i) movimientos++;
		delay = 0;
		while (molecule[molecule_selected-1].y > i) {
			if ((delay % 6) == 0) molecule[molecule_selected-1].y -= 1;
			delay++;
			if (delay == 12) delay = 0;
			DrawScene();
		}
	}
	if (y > 0) {
		i = mapy+1;
		while (map[mapx][i].tile == 101) {
			if (CheckAtom(mapx,i) != 0) break;
			i++;
		}
		i--;
		mapy = i;
		if (molecule[molecule_selected-1].y != i) movimientos++;
		delay = 0;
		while (molecule[molecule_selected-1].y < i) {
			if ((delay % 6) == 0) molecule[molecule_selected-1].y += 1;
			delay++;
			if (delay == 12) delay = 0;
			DrawScene();
		}
	}
}

void MoveAtomOneStep(int x, int y) {
	int i;
	if (x < 0) {
		i = mapx-1;
		if ((map[i][mapy].tile != 101) || (CheckAtom(i,mapy) != 0)) {
			i++;
		}
		mapx = i;
		molecule[molecule_selected-1].x = i;
	}
	if (x > 0) {
		i = mapx+1;
		if ((map[i][mapy].tile != 101) || (CheckAtom(i,mapy) != 0)) {
			i--;
		}
		mapx = i;
		molecule[molecule_selected-1].x = i;
	}
	if (y < 0) {
		i = mapy-1;
		if ((map[mapx][i].tile != 101) || (CheckAtom(mapx,i) != 0)) {
			i++;
		}
		mapy = i;
		molecule[molecule_selected-1].y = i;
	}
	if (y > 0) {
		i = mapy+1;
		if ((map[mapx][i].tile != 101) || (CheckAtom(mapx,i) != 0)) {
			i--;
		}
		mapy = i;
		molecule[molecule_selected-1].y = i;
	}
}

void Editor_InputPAD() {
	sceCtrlReadBufferPositive(&PSP_Pad, 1);
  PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  PSP_Pad_last = PSP_Pad.Buttons;

  if (PSP_Pad_new != 0) {
  	if (molecule_selected == 0) {
			if (PSP_Pad_new & PSP_CTRL_UP) mapy--;
			if (PSP_Pad_new & PSP_CTRL_DOWN) mapy++;
			if (PSP_Pad_new & PSP_CTRL_LEFT) mapx--;
			if (PSP_Pad_new & PSP_CTRL_RIGHT) mapx++;
		} else {
			if (PSP_Pad_new & PSP_CTRL_UP) MoveAtomOneStep(0,-1);
			if (PSP_Pad_new & PSP_CTRL_DOWN) MoveAtomOneStep(0,1);
			if (PSP_Pad_new & PSP_CTRL_LEFT) MoveAtomOneStep(-1,0);
			if (PSP_Pad_new & PSP_CTRL_RIGHT) MoveAtomOneStep(1,0);
		}
		if (PSP_Pad_new & PSP_CTRL_START) game_done=1;
		if (PSP_Pad_new & PSP_CTRL_CROSS) {
			if (molecule_selected == 0) {
				if (CheckAtom(mapx, mapy) != 0) {
					molecule_selected = CheckAtom(mapx, mapy);
				} else {
					if (map[mapx][mapy].tile != 101) {
						map[mapx][mapy].tile = 101;  // 0x65
					} else {
						map[mapx][mapy].tile = 102; // 0x66
					}
				}
			} else {
					molecule_selected = 0;
			}
		}	
		if (PSP_Pad_new & PSP_CTRL_RTRIGGER) {
			if (level < 19) {
				level++;
				LoadLevel();
			}
		}
		if (PSP_Pad_new & PSP_CTRL_LTRIGGER) {
			if (level > 1) {
				level--;
				LoadLevel();
			}
		}
		if (PSP_Pad_new & PSP_CTRL_TRIANGLE) {
			map[mapx][mapy].tile = 100; // 0x64
		}
		if (PSP_Pad_new & PSP_CTRL_SELECT) {
			SaveLevel();
		}
	}
	if (mapy < 0) mapy = 0;
	if (mapy > MAP_HEIGHT-1) mapy = MAP_HEIGHT-1;
	if (mapx < 0) mapx = 0;
	if (mapx > MAP_WIDTH-1) mapx = MAP_WIDTH-1;
}

void StartEditor() {
	level = 1;
	editando = 1;
  LoadLevel();
  game_done = 0;
  while (game_done == 0) {
		Editor_InputPAD();
		DrawScene();
	}
	level = 0;
}

int CheckSolucion() {
	int i, ii, difx=0, dify=0, molecula_ok;
	molecula_ok = 0;
	for (i=0;i<molecules;i++) {
		if (molecule[i].activo == 1) {			
			for (ii=0;ii<molecules;ii++) {
				if((solve[i].link == molecule[ii].link) && (solve[i].dlink == molecule[ii].dlink) && (solve[i].tipo == molecule[ii].tipo)) {
					difx = molecule[ii].x - solve[i].x;
					dify = molecule[ii].y - solve[i].y;
					break;
				}
			}
		}
  }
	for (i=0;i<molecules;i++) {
		if (molecule[i].activo == 1) {			
			for (ii=0;ii<molecules;ii++) {
				if ((solve[i].x+difx == molecule[ii].x) && (solve[i].y+dify == molecule[ii].y)) {
					if((solve[i].link == molecule[ii].link) && (solve[i].dlink == molecule[ii].dlink) && (solve[i].tipo == molecule[ii].tipo)) {
						molecula_ok++;	
					}
				}
			}
		}
	}
	if (molecula_ok == molecules) {
		return 1;
	} else {
		return 0;
	}
}


void NextLevel() {
	int done=0;
	level++;
	if (level == 20) level = 1;
	while (!done) {
		blitImageToScreen(0 ,0 ,480, 272, img_info, 0, 0);
		writeFont(116,100,"CONGRATULATIONS");
		writeFont(116,120,"LEVEL COMPLETED");
		writeFont(116,160,"LEVEL CODE:");
		writeFont(308,160,passw[level]);
		sceCtrlReadBufferPositive(&PSP_Pad, 1);
  	PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  	PSP_Pad_last = PSP_Pad.Buttons;

  	if (PSP_Pad_new != 0) {
			if (PSP_Pad_new & PSP_CTRL_CROSS) done = 1;
		}
		sceDisplayWaitVblankStart(); 
		flipScreen(); 
	}
	
	LoadLevel();
	molecule_selected = 0;
}

void InputPAD() {
	sceCtrlReadBufferPositive(&PSP_Pad, 1);
  PSP_Pad_new = PSP_Pad.Buttons & ~PSP_Pad_last;
  PSP_Pad_last = PSP_Pad.Buttons;

   	if(PSP_Pad.Buttons & PSP_CTRL_LTRIGGER && PSP_Pad.Buttons & PSP_CTRL_RTRIGGER) {
  	 	Screenshot("ms0:/Screenshot00.png",getVramDisplayBuffer(),SCREEN_WIDTH,SCREEN_HEIGHT,PSP_LINE_SIZE,0);
		}
  if (PSP_Pad_new != 0) {
  	if (molecule_selected == 0) {
			if (PSP_Pad_new & PSP_CTRL_UP) mapy--;
			if (PSP_Pad_new & PSP_CTRL_DOWN) mapy++;
			if (PSP_Pad_new & PSP_CTRL_LEFT) mapx--;
			if (PSP_Pad_new & PSP_CTRL_RIGHT) mapx++;
		} else {
			if (PSP_Pad_new & PSP_CTRL_UP) MoveAtom(0,-1);
			if (PSP_Pad_new & PSP_CTRL_DOWN) MoveAtom(0,1);
			if (PSP_Pad_new & PSP_CTRL_LEFT) MoveAtom(-1,0);
			if (PSP_Pad_new & PSP_CTRL_RIGHT) MoveAtom(1,0);
		}
		if (PSP_Pad_new & PSP_CTRL_START) game_done=1;
		if (PSP_Pad_new & PSP_CTRL_CROSS) {
			if (molecule_selected == 0) {
				if (CheckAtom(mapx, mapy) != 0) molecule_selected = CheckAtom(mapx, mapy);
			} else {
					molecule_selected = 0;
			}
		}	
	}
	if (mapy < 0) mapy = 0;
	if (mapy > MAP_HEIGHT-1) mapy = MAP_HEIGHT-1;
	if (mapx < 0) mapx = 0;
	if (mapx > MAP_WIDTH-1) mapx = MAP_WIDTH-1;
}

void StartGame() {
	mapx = 0;
	mapy = 0;
	editando = 0;
  LoadLevel();
  molecule_selected = 0;
  game_done = 0;
  while (game_done == 0) {
		InputPAD();
		DrawScene();
		if (CheckSolucion() == 1) {
				NextLevel();				
		}
	}
	level = 0;
}

// MAIN------------------------------------------------------------------------

int main() {  
  scePowerSetClockFrequency(333, 333, 166);//Subimos a 333mhz. 
  
  pspDebugScreenInit();
  SetupCallbacks();
  initGraphics();
  
  img_main = loadImage("atomix.png");
  blitImageToScreen(0 ,0 ,480, 272, img_main, 0, 0);
	sceDisplayWaitVblankStart(); 
	flipScreen(); 
  LoadGFX();
  sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	level = 0;
	
	while (true) {
		if (level == 0) MainMenu();
		
		switch ( option_selected )
  	  {
    	  case 1:
      	  StartGame();
        	break;	
    	  case 2:
      	  GoLevel();
        	break;	
    	  case 3:
      	  ShowCredits();
        	break;	
    	  case 4:
 	  			StartEditor();
        	break;	
			}
	}
  sceKernelSleepThread();
  return 0;
}

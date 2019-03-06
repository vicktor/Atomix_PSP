/***************************************************************************
 - CSprite.cpp -
 (C) 2003 by Alberto Garcia Serrano
 This software is under GPL licence
 ***************************************************************************
 2006 -  Naboo [PostumuM] http://www.postumum.com
 - Modificado para funcionar con PSP
 - Modificado para cargar todos los frames de un sprite de una sola vez
 ***************************************************************************/

#include "csprite.h"
#include "graphics.h"

// Sprite Class implementation

void CSprite::inicializa(char *path, int nf, int x, int y) {
	img = loadImage(path);	
	nframes=nf;
	frame_actual=0;
	activo=1;
	ciclo=0;
	setx(x);
	sety(y);
}

void CSprite::unload(){
	freeImage(img);
}

void CSprite::selframe(int nf) {
	if (nf<=nframes) {	
		estado=nf;
	}
}

void CSprite::draw() {
	if (activo == 1) {
		blitAlphaImageToScreen(frame_actual * getw(), 0, getw(), geth(), img, posx, posy);
		delay++;
		if (delay == 20) delay = 0;
		if (delay % 5 == 0) frame_actual++;
		if (frame_actual == nframes) {
			frame_actual = 0;
			if (ciclo == 1) activo = 0;
		}
	}
}

int CSprite::colision(CSprite sp) {
int w1,h1,w2,h2,x1,y1,x2,y2;

	w1=getw();			// ancho del sprite1
	h1=geth();			// altura del sprite1
	w2=sp.getw();		// ancho del sprite2
	h2=sp.geth();		// alto del sprite2
	x1=getx();			// pos. X del sprite1
	y1=gety();			// pos. Y del sprite1
	x2=sp.getx();		// pos. X del sprite2
	y2=sp.gety();		// pos. Y del sprite2

	if (((x1+w1)>x2)&&((y1+h1)>y2)&&((x2+w2)>x1)&&((y2+h2)>y1)&&(activo==1)) {
		return TRUE;
	} else {
		return FALSE;
	}
}

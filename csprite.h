/***************************************************************************
 - CSprite.h -
 (C) 2003 by Alberto Garcia Serrano
 This software is under GPL licence
 ***************************************************************************
 2006 -  Naboo [PostumuM] http://www.postumum.com
 - Modificado para funcionar con PSP
 - Modificado para cargar todos los frames de un sprite de una sola vez
 ***************************************************************************/


#ifndef CSPRITE_H_
#define CSPRITE_H_

#define TRUE 1
#define FALSE 0

#include "graphics.h"

class CSprite {
private:
	int posx,posy;
	int estado;
	int nframes;
	int delay;
	int frame_actual;
	int cont;
	int activo;
	int ciclo;
	Image* img;

public:
	void inicializa(char *path, int nf, int x, int y);
	void unload();
	void selframe(int nf);
	int frames() {return nframes;}
	void setx(int x) {posx=x;}
	void sety(int y) {posy=y;}
	void incx(int x) {posx+=x;}
	void incy(int y) {posy+=y;}
	void addx(int c) {posx+=c;}
	void addy(int c) {posy+=c;}
	void setactivo(int i) {activo=i;}
	int getactivo() { return activo;}
	void setciclo(int i) {ciclo=i;}
	int getx() {return posx;}
	int gety() {return posy;}
	int getw() {return img->imageWidth / frames();}
	int geth() {return img->imageHeight;}
	void draw();
	int colision(CSprite sp);
};

#endif /* CSPRITE_H_ */

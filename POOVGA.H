#ifdef _MIKEVGA
#else

#define _MIKEVGA

#include <dos.h>
#include <math.h>
#include <vesa.h>
#include <stdlib.h>
#include <alloc.h>
#include <stdio.h>
#include <conio.h>

#define CIRCLE_PRECISION .005
#define CIRCLE_TABLE	 1280
#define PI 3.1415927
#define MOV_PUT 0
#define XOR_PUT 1

//Global Declarations
void far *vga;			//video segment
int	dimX,dimY;			//x and y dimensions
unsigned char graphicsCol;	//color
unsigned char pall[256][3];	//pallette
int	curPage=0;				//current video page
int maxWins=1;				//max # of windows
int	writeMode=MOV_PUT;		//write mode
void far (*winFuncPtr)();	//pointer to change window interrupt
float *cosTbl=NULL;
float *sinTbl=NULL;

//Screen Initialization Related
int initMCGA(void);
int deinitMCGA(void);
int initSVGA(void);
int deinitSVGA(void);
int initText(void);
void cls(unsigned char col);
void cls(void);

//Drawing Functions
void setPix(int x, int y, unsigned char col);
void setPix( int x, int y);
void line(int x1, int y1,int x2,int y2, unsigned char col);
void line(int x1, int y1,int x2,int y2);
void hline(int x1, int y1, int x2);
void vline(int x1, int y1, int y2);
void circle(int x, int y, int radius, unsigned char col);
void circle(int x, int y, int radius);
void fan(int x, int y, int radius, unsigned char col);
void fan(int x, int y, int radius);
void filledCircle(int x, int y, int radius, unsigned char col);
void filledCircle(int x, int y, int radius);
void rectangle(int x1, int y1, int x2, int y2,unsigned char col);
void rectangle(int x1, int y1, int x2, int y2);
void filledRectangle(int x1, int y1, int x2, int y2, unsigned char col);
void filledRectangle(int x1, int y1, int x2, int y2);
void shape(int numSides,int *ptr);

//Pallette Stuff
void getPal(unsigned char col,unsigned char &R,unsigned char &G,unsigned char &B); //get color
void setPal(unsigned char col,unsigned char R, unsigned char G, unsigned char B);  //set color
void clearPal(void);													//set all to 0
void getPallette(void);													//get whole pallette
void restorePallette(void);
void fadeIn(int del);
void fadeOut(int del);

//Other Stuff
void setWriteMode(int mode)		{ writeMode=mode; };
int getWriteMode(void)					{ return writeMode; };
void waitRetrace(void);
void setColor(unsigned char col)		{ graphicsCol=col; };
unsigned char getColor(void)			{ return graphicsCol;	   };
float abs(float a) 						{ return (a>0 ? a : 0-a);};
int round(float a) 						{ return (abs(a-(int)a)>.5 ? (int)a+1 : (int)a);};
double sqr(double a)					{ return a*a; };

/*************************Initialization Functions********************************/
void cls(unsigned char col) {
	long int a;
	int b;
	
	for (b=0;b<maxWins;b++) {
		setWindow(b,winFuncPtr);
		for(a=0;a<=(long int)0xFFFF;a++) { 
			_ES=FP_SEG(vga);
			_DI=FP_OFF(vga)+a;
			_AL=col;
			asm { STOSB }
		}
	}
}

void cls(void) {
	long int a;
	int b;
	
	for (b=0;b<maxWins;b++) {
		setWindow(b,winFuncPtr);
		for(a=0;a<=(long int)0xFFFF;a++) { 
			_ES=FP_SEG(vga);
			_DI=FP_OFF(vga)+a;
			_AL=0;
			asm { STOSB }
		}
	}
}

int initMCGA(void) {
	float deg;
	int a;
	
	vga=MK_FP(0xA000,0);
	dimX=320;	dimY=200;
	curPage=0;
	asm {	MOV		AX,0x0013
			INT		0x10		}
	
	if (sinTbl==NULL) sinTbl=(float *)malloc(CIRCLE_TABLE*sizeof(float));
	if (cosTbl==NULL) cosTbl=(float *)malloc(CIRCLE_TABLE*sizeof(float));
	if (sinTbl==NULL) {
		printf("sinTbl was null");
		exit(1);
	}
	if (cosTbl==NULL) {
		printf("cosTbl was null");
		exit(1);
	}
	deg=0.0;
	for (a=0;a<=CIRCLE_TABLE;a++,deg+=CIRCLE_PRECISION) {
		cosTbl[a]=cos(deg);
		sinTbl[a]=sin(deg);
	}
	return 1;
}

int initSVGA(void) {
	int a,size;
	float deg;
	struct VESA *vesa=(struct VESA *)malloc(256);
	struct VESAfunc *vesafunc=(struct VESAfunc *)malloc(256);
	
	if (!getVESA(vesa)) return 0;
	if (!checkVESA(vesa)) return 0;
	if (!supportMode(vesa,0x103)) return 0;
	if (!getVESAfunc(vesafunc,0x103)) return 0;
	if (!setMode(0x103)) return 0;
	curPage=0;
	maxWins=8;
	vga=MK_FP(vesafunc->WinASegment,0);
	dimX=vesafunc->XResolution;
	dimY=vesafunc->YResolution;
	winFuncPtr=(void (far*)())(vesafunc->WinFuncPtr);
	
	if (sinTbl==NULL) sinTbl=(float *)malloc(CIRCLE_TABLE*sizeof(float));
	if (cosTbl==NULL) cosTbl=(float *)malloc(CIRCLE_TABLE*sizeof(float));
	if (sinTbl==NULL) {
		printf("sinTbl was null");
		exit(1);
	}
	if (cosTbl==NULL) {
		printf("cosTbl was null");
		exit(1);
	}
	deg=0.0;
	for (a=0;a<=CIRCLE_TABLE;a++,deg+=CIRCLE_PRECISION) {
		cosTbl[a]=cos(deg);
		sinTbl[a]=sin(deg);
	}
	return 1;
}

int deinitSVGA(void) {
	asm {	MOV		AX,0x0003	
			INT		0x10		}
	maxWins=1;
	dimX=0;
	dimY=0;
	return 1;	
}	

int deinitMCGA(void) {
	asm {	MOV		AX,0x0003	
	INT		0x10		}
	maxWins=1;
	dimX=0;
	dimY=0;
	return 1;
}

int initText(void) {
	asm {	MOV		AX,0x0003	
			INT		0x10		}
	maxWins=1;
	dimX=0;
	dimY=0;
	return 1;
}

/*******************************Drawing Functions*******************************/

/*&&&&&&&&&&&&&& C Version Of setPix(int x, int y) - Just for reference &&&&&&&&&&&&&
void setPix2(long int x, long int y) {
	long int temp;
	if ( (x >= dimX) || (x < 0) || (y >= dimY) || (y < 0) )	return;   //bounds checking
	temp=(long int)((x+(y*dimX))>>16);						  //get window of this pixel
//	printf("x: %li y: %li dimx: %li granulation: %li y*dimX: %li x+(y*dimX): %li ans: %li curpage: %li\n",x,y,dimX,granulation,y*dimX,x+(y*dimX),temp,(long int)curPage);
	if(temp!=(long int)(curPage)) {			//if not in the right window, change windows
		setWindow(temp,winFuncPtr);			//change window	
		curPage=temp;						//change global variable-so this function isn't 
	}										//called 1000 times when there's no need

	_ES=FP_SEG(vga);						//load up ES:DI with address of video seg
	_DI=FP_OFF(vga)+((y * (long int)(dimX)) + x)-(temp*0xFFFF)-temp;
	_AL=graphicsCol;						//global variable for current color
	asm { STOSB }							//set pixel
}*/

void setPix(int x, int y, unsigned char col) {
	asm {
		PUSH	AX					//push ax no matter what
		MOV		AX,x				//put x dimension in register for bounds check
		CMP		AX,dimX				//too big?
		JGE		baddim				//return
		CMP		AX,0				//too small?
		JL		baddim
		MOV		AX,y				//set up y dimension for bounds check
		CMP		AX,dimY				//too big?
		JGE		baddim
		CMP		AX,0				//too small?
		JLE		good
baddim:
		POP		AX					//pop only ax
		JMP		setpixdone
good:
		PUSH	BX
		PUSH	CX
		PUSH	DX					//save registers
		PUSH	DI
		PUSH	SI
		MOV		AX,dimX				//computer (x+(y*dimX))/granulation to see 
		MUL		y					//if we are already in the right window
		ADD		AX,x
		ADC		DX,0
        CMP     DX,curPage			//check if in right window
        JZ      iscurpage			//if so go to line 187
		MOV		curPage,DX			//window to go to
		XOR		BX,BX				//set window, window a
		CALL	DWORD [winFuncPtr]	//call interrupt
iscurpage:
 		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y					//well, this part computes (x+(y*dimX))
		ADD		AX,x
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		SBB		DX,0				//subtract 1 if borrow
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga		//put vga seg in ES:DI
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		AL,col				//al=color
		CMP		writeMode,XOR_PUT	//check if writeMode=XOR
		JZ		isxor				//if so, go to that code
		STOSB						//writemode is MOV_PUT, so store the byte
		JMP		finish_setPixel		//finish up
isxor:
		XOR		[ES:DI],AL			//in XOR mode, so XOR the byte
finish_setPixel:
		POP		SI
		POP		DI
		POP		DX
		POP		CX					//restore registers
		POP		BX
		POP		AX
	}
setpixdone:
}

void setPix(int x, int y) {
	asm {
		PUSH	AX					//push ax no matter what
		MOV		AX,x				//put x dimension in register for bounds check
		CMP		AX,dimX				//too big?
		JGE		baddim				//return
		CMP		AX,0				//too small?
		JL		baddim
		MOV		AX,y				//set up y dimension for bounds check
		CMP		AX,dimY				//too big?
		JGE		baddim
		CMP		AX,0				//too small?
		JGE		good
baddim:
		POP		AX					//pop only ax
		JMP		setpixdone			//get the hell out
good:
		PUSH	BX
		PUSH	CX
		PUSH	DX					//save registers
		PUSH	DI
		PUSH	SI
		MOV		AX,dimX				//computer (x+(y*dimX))/granulation to see 
		MUL		y					//if we are already in the right window
		ADD		AX,x
		ADC		DX,0
        CMP     DX,curPage			//check if in right window
        JZ      iscurpage			//if so go to line 187
		MOV		curPage,DX			//window to go to
		XOR		BX,BX				//set window, window a
		CALL	DWORD [winFuncPtr]	//call interrupt
iscurpage:
 		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y					//well, this part computes (x+(y*dimX))
		ADD		AX,x
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		SBB		DX,0				//subtract 1 if borrow
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga		//put vga seg in ES:DI
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		AL,graphicsCol		//al=color
		CMP		writeMode,XOR_PUT	//check if writeMode=XOR
		JZ		isxor				//if so, go to that code
		STOSB						//writemode is MOV_PUT, so store the byte
		JMP		finish_setPixel		//finish up
isxor:
		XOR		[ES:DI],AL			//in XOR mode, so XOR the byte
finish_setPixel:
		POP		SI
		POP		DI
		POP		DX
		POP		CX					//restore registers
		POP		BX
		POP		AX
	}
	setpixdone:
}

void vline(int x1,int y1,int y2) {
	int gapSize=dimX-1;
	
	if ( (y1==y2) || (x1>dimX) || (y2>dimY) || (x1<0) || (y2<0) || (y1<0) || (y1>dimY)) return;
	
	asm{
		MOV		AX,y1				//make sure y1 is smaller than y2
		CMP		AX,y2
		JLE		okvlinedims			
		MOV		BX,y2				//points are backwards
		MOV		y1,BX
		MOV		y2,AX
okvlinedims:
		MOV		AX,dimX				//compute (x+(y*dimX))/granulation to see 
		MUL		y1					//if we are already in the right window
		ADD		AX,x1
		ADC		DX,0
	}
	if (_DX!=curPage) {
		asm {
			MOV		curPage,DX
			XOR		BX,BX
			CALL	DWORD [winFuncPtr]
		}
	}
	asm {
		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y1					//well, this part computes (x+(y*dimX))
		ADD		AX,x1
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		CX,y2				//loop y2-y1+1 times
		SUB		CX,y1
		INC		CX
		MOV		AL,graphicsCol		//al=color
loopdeloop:
		PUSH	CX
		PUSH	AX					//save color
		MOV		AX,writeMode
		CMP		AX,MOV_PUT			//check what type of store byte
		JNZ		notnotmovput
		POP AX						//mov the color there
		STOSB
		JMP		endstorebyte
notnotmovput: 
		POP	AX
		XOR [ES:DI],AL				//xor the color there
		INC DI
endstorebyte:						//continue
		MOV  	BX,DI
		ADD  	BX,gapSize			//check if we'll need to change window
		JC  	changedewindow
		JMP	  	nochangedewindow
changedewindow:						//yes, change window
		PUSH 	ES
		PUSH	AX
		INC	 	curPage
		MOV		DX,curPage
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
		POP		AX
		POP		ES
nochangedewindow:
	 	ADD 	DI,gapSize				//next byte
		POP		CX
		LOOP	loopdeloop
	 }	
}

void hline(int x1,int y1,int x2) {
	if ( (x1==x2) || (x1>dimX) || (x2>dimX) || (x1<0) || (x2<0) || (y1<0) || (y1>dimY)) return;
	
	asm{
		MOV		AX,x1				//make sure y1 is smaller than y2
		CMP		AX,x2
		JLE		okhlinedims			
		MOV		BX,x2				//points are backwards
		MOV		x1,BX
		MOV		x2,AX
okhlinedims:
		MOV		AX,dimX				//computer (x+(y*dimX))/granulation to see 
		MUL		y1					//if we are already in the right window
		ADD		AX,x1
		ADC		DX,0
		CMP		DX,curPage			//check if we're in the right window
		JZ		yeprightpage		//if so carry on
		MOV		curPage,DX			//er else change window
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
yeprightpage:
		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y1					//well, this part computes (x+(y*dimX))
		ADD		AX,x1
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		CX,x2				//loop y2-y1+1 times
		SUB		CX,x1
		INC		CX
		MOV		AL,graphicsCol		//al=color
hlinelooper39:
		PUSH	CX
		PUSH	AX
		MOV		AX,writeMode
		CMP		AX,MOV_PUT
		JNZ		nopeweusexorput
		POP AX
		STOSB
		JMP		weknowwhatsup
nopeweusexorput:
		POP	AX
		XOR [ES:DI],AL
		INC	DI
weknowwhatsup:	
		AND		DI,DI				//check if we need to go to the next window
		JNZ		noperdoper			//no
		PUSH 	ES					//yes, so..
		PUSH	AX
		INC	 	curPage				//increment current window
		MOV		DX,curPage			//put it into DX
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]	//call up the function
		XOR		DI,DI				//start at offset 0
		POP		AX
		POP		ES
noperdoper:
		POP		CX
		LOOP	hlinelooper39
	}
}

void line(int x1, int y1, int x2, int y2) {
	int xdist,ydist,d,incsame,incdiff,x,y;
	
	xdist=x2-x1;
	ydist=y2-y1;
	
	if (xdist==0) {
		vline(x1,y1,y2);
		return;
	} else if (ydist==0) {
		hline(x1,y1,x2);
		return;
	}
	
	if(abs(xdist)>abs(ydist)) {				// X-Axis is major axis
		if (x1<x2) {						// x looping forwards (increasing)
			if (y1<y2) {					// y is increasing
				incsame=ydist << 1;			// positive
				d=incsame-xdist;			// negative
				incdiff=(ydist-xdist) << 1; // negative

				for(x=x1,y=y1;x<=x2;x++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						y++;
					}
					setPix(x,y);
				}
			} else {						// y is decreasing
				incsame=ydist << 1;			// negative
				d=incsame+xdist;			// positive
				incdiff=(ydist+xdist) << 1;	// positive

				for(x=x1,y=y1;x<=x2;x++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						y--;
					}
					setPix(x,y);
				}
			}
		} else {							// x is looping backwards (decreasing)
			if (y1<y2) {					// y is increasing
				incsame=(-ydist) << 1;		// negative
				d=incsame-xdist;			// positive
				incdiff=(-ydist-xdist) << 1;	// positive
				
				for(x=x2,y=y2;x<=x1;x++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						y--;
					}
					setPix(x,y);
				}
			} else {							// y is decreasing
				incsame=(-ydist) << 1;			// positive
				d=ydist+xdist;					// negative
				incdiff=(-ydist+xdist) << 1; 	// negative
				
				for(x=x2,y=y2;x<=x1;x++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						y++;
					}
					setPix(x,y);
				}
			}
		}	
	} else {								// Y-Axis is major axis
		if (y1<y2) {						// y looping forwards (increasing)
			if (x1<x2) {					// x is increasing
				incsame=xdist << 1;			// positive
				d=incsame-ydist;			// negative
				incdiff=(xdist-ydist) << 1; // negative

				for(y=y1,x=x1;y<=y2;y++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						x++;
					}
					setPix(x,y);
				}
			} else {						// x is decreasing
				incsame=xdist << 1;			// negative
				d=incsame+ydist;			// positive
				incdiff=(xdist+ydist) << 1;	// positive

				for(y=y1,x=x1;y<=y2;y++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						x--;
					}
					setPix(x,y);
				}
			}
		} else {							// y is looping backwards (decreasing)
			if (x1<x2) {					// x is increasing
				incsame=(-xdist) << 1;		// negative
				d=incsame-ydist;			// positive
				incdiff=(-xdist-ydist) << 1;	// positive
				
				for(y=y2,x=x2;y<=y1;y++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						x--;
					}
					setPix(x,y);
				}
			} else {							// x is decreasing
				incsame=(-xdist) << 1;			// positive
				d=xdist+ydist;					// negative
				incdiff=(-xdist+ydist) << 1; 	// negative
				
				for(y=y2,x=x2;y<=y1;y++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						x++;
					}
					setPix(x,y);
				}
			}
		}
	}
}

void line(int x1, int y1, int x2, int y2,unsigned char col) {
	int xdist,ydist,d,incsame,incdiff,x,y;
	unsigned char tempcol=getColor();
	setColor(col);
	
	xdist=x2-x1;
	ydist=y2-y1;
	
	if (xdist==0) {
		vline(x1,y1,y2);
		return;
	} else if (ydist==0) {
		hline(x1,y1,x2);
		return;
	}
	
	if(abs(xdist)>abs(ydist)) {				// X-Axis is major axis
		if (x1<x2) {						// x looping forwards (increasing)
			if (y1<y2) {					// y is increasing
				incsame=ydist << 1;			// positive
				d=incsame-xdist;			// negative
				incdiff=(ydist-xdist) << 1; // negative

				for(x=x1,y=y1;x<=x2;x++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						y++;
					}
					setPix(x,y);
				}
			} else {						// y is decreasing
				incsame=ydist << 1;			// negative
				d=incsame+xdist;			// positive
				incdiff=(ydist+xdist) << 1;	// positive

				for(x=x1,y=y1;x<=x2;x++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						y--;
					}
					setPix(x,y);
				}
			}
		} else {							// x is looping backwards (decreasing)
			if (y1<y2) {					// y is increasing
				incsame=(-ydist) << 1;		// negative
				d=incsame-xdist;			// positive
				incdiff=(-ydist-xdist) << 1;	// positive
				
				for(x=x2,y=y2;x<=x1;x++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						y--;
					}
					setPix(x,y);
				}
			} else {							// y is decreasing
				incsame=(-ydist) << 1;			// positive
				d=ydist+xdist;					// negative
				incdiff=(-ydist+xdist) << 1; 	// negative
				
				for(x=x2,y=y2;x<=x1;x++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						y++;
					}
					setPix(x,y);
				}
			}
		}	
	} else {								// Y-Axis is major axis
		if (y1<y2) {						// y looping forwards (increasing)
			if (x1<x2) {					// x is increasing
				incsame=xdist << 1;			// positive
				d=incsame-ydist;			// negative
				incdiff=(xdist-ydist) << 1; // negative

				for(y=y1,x=x1;y<=y2;y++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						x++;
					}
					setPix(x,y);
				}
			} else {						// x is decreasing
				incsame=xdist << 1;			// negative
				d=incsame+ydist;			// positive
				incdiff=(xdist+ydist) << 1;	// positive

				for(y=y1,x=x1;y<=y2;y++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						x--;
					}
					setPix(x,y);
				}
			}
		} else {							// y is looping backwards (decreasing)
			if (x1<x2) {					// x is increasing
				incsame=(-xdist) << 1;		// negative
				d=incsame-ydist;			// positive
				incdiff=(-xdist-ydist) << 1;	// positive
				
				for(y=y2,x=x2;y<=y1;y++) {
					if (d>0) d+=incsame;
					else {
						d+=incdiff;
						x--;
					}
					setPix(x,y);
				}
			} else {							// x is decreasing
				incsame=(-xdist) << 1;			// positive
				d=xdist+ydist;					// negative
				incdiff=(-xdist+ydist) << 1; 	// negative
				
				for(y=y2,x=x2;y<=y1;y++) {
					if (d<0) d+=incsame;
					else {
						d+=incdiff;
						x++;
					}
					setPix(x,y);
				}
			}
		}
	}
	setColor(tempcol);
}

void circle(int x, int y, int radius, unsigned char col) {
	unsigned char tempCol=getColor();
	setColor(col);
	for(int deg=0;deg<=CIRCLE_TABLE;deg++)
		         setPix(round(radius*cosTbl[deg]+x),round(radius*sinTbl[deg]+y));
	setColor(tempCol);
}

void circle(int x, int y, int radius) {
	for(int deg=0;deg<=CIRCLE_TABLE;deg++)
		         setPix(round(radius*cosTbl[deg]+x),round(radius*sinTbl[deg]+y));
}


void fan(int x, int y, int radius, unsigned char col) {
	int deg;
	unsigned char tempCol=getColor();
	setColor(col);
	for(deg=0;deg<=CIRCLE_TABLE;deg++)
		line(x,y,round(radius*cosTbl[deg]+x),round(radius*sinTbl[deg]+y));
	setColor(tempCol);
}

void fan(int x, int y, int radius) {
	int deg;
	for(deg=0;deg<=CIRCLE_TABLE;deg++)
		line(x,y,round(radius*cosTbl[deg]+x),round(radius*sinTbl[deg]+y));
}

void filledCircle(int x, int y, int radius) {
	int x1,x2=x+radius,y1=y-radius,y2=y+radius;
	for(;y1<=y2;y1++)
		for(x1=x-radius;x1<=x2;x1++)
			if (sqrt(sqr(x-x1)+sqr(y-y1))<=(int)radius) setPix(x1,y1);
}

void filledCircle(int x, int y, int radius,unsigned char col) {
	unsigned char tempcol=getColor();
	setColor(col);
	int x1,x2=x+radius,y1=y-radius,y2=y+radius;
	for(;y1<=y2;y1++)
		for(x1=x-radius;x1<=x2;x1++)
			if (sqrt(sqr(x-x1)+sqr(y-y1))<=radius) setPix(x1,y1);
	setColor(tempcol);
}

void rectangle(int x1, int y1, int x2, int y2,unsigned char col) {
	unsigned char tempcol=getColor();
	setColor(col);

	hline(x1,y1,x2);
	vline(x2,y1,y2);
	hline(x2,y2,x1);
	vline(x1,y2,y1);
	setColor(tempcol);
}

void rectangle(int x1, int y1, int x2, int y2) {
	hline(x1,y1,x2);
	vline(x2,y1,y2);
	hline(x2,y2,x1);
	vline(x1,y2,y1);
}

void filledRectangle(int x1, int y1, int x2, int y2, unsigned char col) {
	int gapSize;
	unsigned char tempcol=getColor();
	setColor(col);
	
	asm{
		MOV		AX,y1
		CMP		AX,y2
		JNZ		y1notequaly2
		JMP		endfilledrectangle	//make sure y1!=y2
y1notequaly2:
		MOV		AX,x1				//make sure x1 is smaller than x2
		CMP		AX,x2
		JB		okvlinedims			
		JA		badfilleddims
		JMP		endfilledrectangle	//if x1=x2 then return
badfilleddims:
		MOV		AX,x1
		MOV		BX,x2				//points are backwards
		MOV		x1,BX
		MOV		x2,AX
		MOV		AX,y1
		MOV		BX,y2
		MOV		y1,BX
		MOV		y2,AX
okvlinedims:
		MOV		AX,x2				//x2-x1
		SUB		AX,x1
		INC		AX					//(x2-x1+1)
		MOV		BX,dimX
		SUB		BX,AX				//dimX-(x2-x1)-1
		MOV		gapSize,BX			//put it in gapSize
		MOV		AX,dimX				//computer (x+(y*dimX))/granulation to see 
		MUL		y1					//if we are already in the right window
		ADD		AX,x1
		ADC		DX,0
		CMP		DX,curPage			//check if we're in the right window
		JZ		yeprightpage		//if so carry on
		MOV		curPage,DX			//er else change window
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
yeprightpage:
		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y1					//well, this part computes (x+(y*dimX))
		ADD		AX,x1
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		CX,y2				//outer loop y2-y1+1 times (number of vertical lines)
		SUB		CX,y1
		INC		CX
outerfilledloop:
		PUSH	CX
		MOV		CX,x2				//inner loop x2-x1+1 times in inner loop (horizontal lines)
		SUB		CX,x1
		INC		CX
		MOV		AL,graphicsCol		//al=color
innerfilledloop:
		PUSH	CX
		PUSH	AX
		MOV		AX,writeMode
		CMP		AX,MOV_PUT
		JNZ		nopeweusexorput
		POP AX
		STOSB
		JMP		weknowwhatsup
nopeweusexorput:
		POP	AX
		XOR [ES:DI],AL
		INC	DI
weknowwhatsup:	
		AND		DI,DI				//check if we need to go to the next window
		JNZ		noperdoper			//no
		PUSH 	ES					//yes, so..
		PUSH	AX
		INC	 	curPage				//increment current window
		MOV		DX,curPage			//put it into DX
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]	//call up the function
		XOR		DI,DI				//start at offset 0
		POP		AX
		POP		ES
noperdoper:
		POP		CX
		LOOP	innerfilledloop
		MOV  	BX,DI
		ADD  	BX,gapSize			//check if we'll need to change window
		JC  	changedewindow
		JMP	  	nochangedewindow
changedewindow:						//yes, change window
		PUSH 	ES
		PUSH	AX
		INC	 	curPage
		MOV		DX,curPage
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
		POP		AX
		POP		ES
nochangedewindow:
	 	ADD 	DI,gapSize				//next byte
		POP		CX
		LOOP	outerfilledloop
endfilledrectangle:
	}
	setColor(tempcol);
}

void filledRectangle(int x1, int y1, int x2, int y2) {
	int gapSize;
	
	asm{
		MOV		AX,y1
		CMP		AX,y2
		JNZ		y1notequaly2
		JMP		endfilledrectangle	//make sure y1!=y2
y1notequaly2:
		MOV		AX,x1				//make sure x1 is smaller than x2
		CMP		AX,x2
		JB		okvlinedims			
		JA		badfilleddims
		JMP		endfilledrectangle	//if x1=x2 then return
badfilleddims:
		MOV		AX,x1
		MOV		BX,x2				//points are backwards
		MOV		x1,BX
		MOV		x2,AX
		MOV		AX,y1
		MOV		BX,y2
		MOV		y1,BX
		MOV		y2,AX
okvlinedims:
		MOV		AX,x2				//x2-x1
		SUB		AX,x1
		INC		AX					//(x2-x1+1)
		MOV		BX,dimX
		SUB		BX,AX				//dimX-(x2-x1)-1
		MOV		gapSize,BX			//put it in gapSize
		MOV		AX,dimX				//computer (x+(y*dimX))/granulation to see 
		MUL		y1					//if we are already in the right window
		ADD		AX,x1
		ADC		DX,0
		CMP		DX,curPage			//check if we're in the right window
		JZ		yeprightpage		//if so carry on
		MOV		curPage,DX			//er else change window
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
yeprightpage:
		MOV		AX,dimX				//compute (x+(y*dimX))-(win*granulation+win)
		MUL		y1					//well, this part computes (x+(y*dimX))
		ADD		AX,x1
		ADC		DX,0				//add if carry
		PUSH	DX					//save result
		PUSH	AX
		MOV		AX,curPage			//compute win*granulation+win to DX:AX
		MOV		BX,0xFFFF
		MUL		BX
		ADD		AX,curPage
		POP		BX 					//put first result in BX:CX
		POP		CX
		SUB		BX,AX				//result in BX
		MOV		SI,OFFSET vga
		LODSW
		MOV		DI,AX
		LODSW
		MOV		ES,AX
		ADD		DI,BX				//get offset of pixel
		MOV		CX,y2				//outer loop y2-y1+1 times (number of vertical lines)
		SUB		CX,y1
		INC		CX
outerfilledloop:
		PUSH	CX
		MOV		CX,x2				//inner loop x2-x1+1 times in inner loop (horizontal lines)
		SUB		CX,x1
		INC		CX
		MOV		AL,graphicsCol		//al=color
innerfilledloop:
		PUSH	CX
		PUSH	AX
		MOV		AX,writeMode
		CMP		AX,MOV_PUT
		JNZ		nopeweusexorput
		POP AX
		STOSB
		JMP		weknowwhatsup
nopeweusexorput:
		POP	AX
		XOR [ES:DI],AL
		INC	DI
weknowwhatsup:	
		AND		DI,DI				//check if we need to go to the next window
		JNZ		noperdoper			//no
		PUSH 	ES					//yes, so..
		PUSH	AX
		INC	 	curPage				//increment current window
		MOV		DX,curPage			//put it into DX
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]	//call up the function
		XOR		DI,DI				//start at offset 0
		POP		AX
		POP		ES
noperdoper:
		POP		CX
		LOOP	innerfilledloop
		MOV  	BX,DI
		ADD  	BX,gapSize			//check if we'll need to change window
		JC  	changedewindow
		JMP	  	nochangedewindow
changedewindow:						//yes, change window
		PUSH 	ES
		PUSH	AX
		INC	 	curPage
		MOV		DX,curPage
		XOR		BX,BX
		CALL	DWORD [winFuncPtr]
		POP		AX
		POP		ES
nochangedewindow:
	 	ADD 	DI,gapSize				//next byte
		POP		CX
		LOOP	outerfilledloop
endfilledrectangle:
	}
}
		

void shape(int numSides,int *ptr,unsigned char col) {
	int a;//  a<6             0,2,4,
	for (a=0;a<2*(numSides-1);a+=2) 
		line(ptr[a],ptr[a+1],ptr[a+2],ptr[a+3],col);
	line(ptr[0],ptr[1],ptr[2*(numSides-1)],ptr[1+2*(numSides-1)],col);
}

/****************************Pallette Functions*****************************/
void getPal(unsigned char col,unsigned char &R,unsigned char &G,unsigned char &B) {
	_DX=0x3C7;
	_AL=col;
	asm	{		OUT		DX,AL	}
	_DX=0x3C9;
	asm	{		IN		AL,DX	}
	R=_AL;
	asm {		IN		AL,DX	}
	G=_AL;
	asm {		IN		AL,DX	}
	B=_AL;
}

void setPal(unsigned char col,unsigned char R, unsigned char G, unsigned char B) {
	_DX=0x3C8;
	_AL=col;
	asm {		OUT		DX,AL	}
	_AL=R;
	_DX=0x3C9;
	asm {		OUT		DX,AL	}
	_AL=G;
	asm {		OUT		DX,AL	}
	_AL=B;
	asm {		OUT		DX,AL	}
}

void clearPal(void) {
	int a;
	for (a=0;a<256;a++)
		setPal(a,0,0,0);
}

void getPallette(void) {
	int a;
	for (a=0;a<256;a++)
		getPal(a,pall[a][0],pall[a][1],pall[a][2]);
}

void fadeIn(int del) {
	int a,b;
	unsigned char temp[3];
	for(a=0;a<64;a++) {
		if(del) delay(del);
		waitRetrace();
		for(b=0;b<256;b++) {
			getPal(b,temp[0],temp[1],temp[2]);
			if ( (temp[0]<pall[b][0]) && (temp[0]<64)) temp[0]++;
			if ( (temp[1]<pall[b][1]) && (temp[1]<64)) temp[1]++;
			if ( (temp[2]<pall[b][2]) && (temp[2]<64)) temp[2]++;
			setPal(b,temp[0],temp[1],temp[2]);
		}
	}
}

void fadeOut(int del) {
	int a,b;
	unsigned char temp[3];
	for(a=63;a>=0;a--) {
		if(del) delay(del);
		waitRetrace();
		for(b=0;b<256;b++) {
			getPal(b,temp[0],temp[1],temp[2]);
			if (temp[0]>0) temp[0]--;
			if (temp[1]>0) temp[1]--;
			if (temp[2]>0) temp[2]--;
			setPal(b,temp[0],temp[1],temp[2]);
		}
	}
}

void restorePallette(void) {
	for (int a=0;a<256;a++)
		setPal(a,pall[a][0],pall[a][1],pall[a][2]);
}


/******************************Other Functions******************************/
void waitRetrace() {
  _DX = 0x03DA;

  l1: asm {
	in  al,dx;
	and al,0x08;
	jnz l1;
  }

  l2: asm {
	in  al,dx;
	and al,0x08;
	jz  l2;
  }
}
#endif

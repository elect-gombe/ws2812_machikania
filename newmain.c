/*
 * File:   newmain.c
 * Author: gombe
 *
 * Created on 2017/01/11, 19:40
 */


#include <xc.h>

#define _SUPPRESS_PLIB_WARNING
#include <plib.h>

#include <stdint.h>

//???????? with PLL (15?)
#pragma config PMDL1WAY = OFF, IOL1WAY = OFF
#pragma config FPLLIDIV = DIV_1, FPLLMUL = MUL_15, FPLLODIV = DIV_1
#pragma config FNOSC = PRIPLL, FSOSCEN = OFF, POSCMOD = XT, OSCIOFNC = OFF
#pragma config FPBDIV = DIV_1, FWDTEN = OFF, JTAGEN = OFF, ICESEL = ICS_PGx1

#define LEDNUM 10

uint8_t ws2812_dat[LEDNUM*8*3+1]={
    0
};


typedef struct{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

color_t col_mul256(const color_t *col,unsigned int n);
color_t color_lerp256(const color_t *c1,const color_t *c2,unsigned int n);
color_t col_add2(const color_t *c1,const color_t *c2);
void Patern_2(unsigned int position,int lng,const color_t *col);

color_t leddata[LEDNUM];

const color_t RED_C = {
    255,0,0,
};
const color_t GREEN_C = {
    0,255,0,
};
const color_t BLUE_C = {
    0,0,255,
};
const color_t CYAN_C = {
    0,255,255,
};

void main(void) {
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_15, OSC_PLL_POST_1, 0);
        
    RPB13R = 5; //RPB13???OC4?????
    OC4R = 0;
    OC4CON = 0x000e; // Timer3????PWM???
    OC4CONSET = 0x8000; //OC4????
    T3CON = 0x0000; // ??????1:1
    PR3 = 54;
    T3CONSET = 0x8000; // ???3????

    DmaChnOpen(0, 0, DMA_OPEN_DEFAULT);

    DmaChnSetEventControl(0, DMA_EV_START_IRQ(_TIMER_3_IRQ));

    DmaChnSetTxfer(0, ws2812_dat, (void*) &OC4RS, sizeof (ws2812_dat), 1, 1);
    
    int i;
    uint8_t t;
    uint8_t f;
    color_t c;
    
    while(1){
        for(i=0;i<LEDNUM*3;i++){
            c = color_lerp256(&RED_C,&CYAN_C,t);
            if(f){
                t+=3;
                if(t > 250)f=0;
            }else{
                t-=3;
                if(t < 5)f=1;
            }
            Patern_2(i,4,&c);
            LEDTrans();
            LEDclear();
            volatile int j;
            for(j=0;j<100000;j++);
        }
        
    }
    
    return;
}

void LEDTrans(void){
    int i;

    for(i=0;i<LEDNUM;i++){
        setBri(i,leddata[i]);
    }

    DmaChnEnable(0);    
}

void LEDclear(void){
    int i;

    for(i=0;i<LEDNUM;i++){
        leddata[i].r = 0;
        leddata[i].g = 0;
        leddata[i].b = 0;
    }
}

void setBri(unsigned int idx,color_t col){
    int i;
    idx *= 8*3;
    
    for(i=0;i<8;i++){
        if(col.r&0x80){
            ws2812_dat[idx++] = 40;
        }else{
            ws2812_dat[idx++] = 20;            
        }
        col.r<<=1;
    }
    for(i=0;i<8;i++){
        if(col.g&0x80){
            ws2812_dat[idx++] = 40;
        }else{
            ws2812_dat[idx++] = 20;            
        }
        col.g<<=1;
    }
    for(i=0;i<8;i++){
        if(col.b&0x80){
            ws2812_dat[idx++] = 40;
        }else{
            ws2812_dat[idx++] = 20;            
        }
        col.b<<=1;
    }
}

color_t col_mul256(const color_t *col,unsigned int n){
    color_t c;

    c.r = (col->r * n) >> 8;
    c.g = (col->g * n) >> 8;
    c.b = (col->b * n) >> 8;

    return c;
}

color_t color_lerp256(const color_t *c1,const color_t *c2,unsigned int n){
    color_t c,ct;
    c = col_mul256(c1,n);
    ct = col_mul256(c2,256-n);
    
    return col_add2(&c,&ct);
}


color_t col_add2(const color_t *c1,const color_t *c2){
    color_t c;

    c.r = c1->r+c2->r;
    c.g = c1->g+c2->g;
    c.b = c1->b+c2->b;
    
    return c;
}

void Patern_2(unsigned int position,int lng,const color_t *col){
  unsigned int i;
  int j;
  unsigned int k;

  k = (lng * 9);  
  for(i=1;i<lng*3;i++){
    j = ((unsigned int)(position + lng * 3 - i + (LEDNUM*3)) % (LEDNUM*3))/3;
    color_t color = col_mul256(col,i*256/k);
    leddata[j] = col_add2(&leddata[j],&color);
  }

  for(i=lng*3;i>=1;i--){
    j = ((unsigned int)(position + i - lng * 3 + (LEDNUM*3)) % (LEDNUM*3))/3;
    color_t color = col_mul256(col,i*256/k);
    leddata[j] = col_add2(&leddata[j],&color);
  }
}

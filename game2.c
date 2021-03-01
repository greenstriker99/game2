/*
Read from controllers with pad_poll().
We also demonstrate sprite animation by cycling among
several different metasprites, which are defined using macros.
*/

#include <stdlib.h>
#include <string.h>

// include NESLIB header
#include "neslib.h"
//#link "obj_background.s"


// include CC65 NES Header (PPU)
#include <nes.h>


extern char effects[];
extern const byte obj_background_pal[16];
extern const byte obj_background_rle[];



// link the pattern table into CHR ROM
//#link "chr_generic.s"

///// METASPRITES

// define a 2x2 metasprite
// define a 2x2 metasprite
#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
        0,      0,      (code)+0,   pal, \
        0,      8,      (code)+1,   pal, \
        8,      0,      (code)+2,   pal, \
        8,      8,      (code)+3,   pal, \
        128};

// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};


// define a 2x2 metasprite, flipped horizontally
#define DEF_METASPRITE_B_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};

DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);
DEF_METASPRITE_2x2(playerRJump, 0xe8, 0);
DEF_METASPRITE_2x2(playerRClimb, 0xec, 0);
DEF_METASPRITE_2x2(playerRSad, 0xf0, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);
DEF_METASPRITE_2x2_FLIP(playerLJump, 0xe8, 0);
DEF_METASPRITE_2x2_FLIP(playerLClimb, 0xec, 0);
DEF_METASPRITE_2x2_FLIP(playerLSad, 0xf0, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);




const unsigned char* const playerRunSeq[16] = {
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2,
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2,
};

/*{pal:"nes",layout:"nes"}*/
const char PALETTE[22] = { 
  0x11,			// screen color

 

  0x06,0x0,0x25,0x0,	// background palette 3

  0x16,0x35,0x25,0x0,	// sprite palette 0
  0x00,0x37,0x24,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x3a	// sprite palette 3
};

// setup PPU and tables
void setup_graphics() {
  // clear sprites
  oam_hide_rest(0);
  // set palette colors
  pal_all(PALETTE);
  // turn on PPU
  ppu_on_all();
}

// number of actors (4 h/w sprites each)
#define NUM_ACTORS 2
#define NUM_objs 15		
// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];



typedef struct obj{
  bool falling;
  byte _x;		// object x/y position
  byte _y;		
  sbyte _dx;		// object x/y deltas per frame (signed)
  sbyte _dy;
  int sprite;
  int points;
  
};


byte rndint(byte, byte);
byte iabs(int x);
void obj_collision(int,);

byte rndint(byte a, byte b){
  return (rand() % (b-a)) + a;
}

struct obj objs[10];
int score;

//title screen
int title()
{
  ppu_off();
pal_col(0,0x02);	// set screen to dark blue
  pal_col(1,0x14);	// fuchsia
  pal_col(2,0x20);	// grey
  pal_col(3,0x30);
  vram_adr(NTADR_A(6, 8));
  vram_write("PRESS START", 11);
  
  vram_adr(NTADR_A(6, 12));
  vram_write("ENTER for 1 Player", 18);
  
  vram_adr(NTADR_A(6, 14));
  vram_write("UP for 2 Players", 16);
  
  ppu_on_all();
  //Start for 1 player
   while(1){     
     if(pad_trigger(0)&PAD_START)
     {return 1;
       //break;
     }
     //UP for 2 player
     else if(pad_trigger(0)&PAD_UP)
     {
       return 2;
       //break;
     }
    }
  
  
}
// main program
void main() {
  
   int choice;

  char i; // actor index
  char oam_id;	// sprite ID
  char pad;	// controller flags
  score = 0;
  
  
  //title screen function returns users choice for 1 or 2 player
  choice = title();
  

   // Initialize actor objects
  for(i=0;i<10;i++){		
    objs[i].falling = false;		//Controls when object falls
    objs[i]._x = rndint(20,230);	//X position
    objs[i]._y = rndint(15,70);		//Y position
    objs[i]._dy = 1;			//Falling Speed
    objs[i].sprite = i+22;		//Sprite used
    objs[i].points = i+1;		//Points added when collected
  }
  
   setup_graphics();

    actor_x[0] = 120;
    actor_y[0] = 191;
    actor_dx[0] = 0;
    actor_dy[0] = 0;
  
  // print instructions
  ppu_off();
  //write empty spaces to cover menu screen words
  
  vram_adr(NTADR_A(6, 8));
  vram_write("           ", 11);
  
  vram_adr(NTADR_A(6, 12));
  vram_write("                  ", 18);
  
  vram_adr(NTADR_A(6, 14));
  vram_write("                ", 16);
  
  vram_adr(NTADR_A(6,2));
  vram_write("DODGE THE OBJECTS", 17);
  //vram_adr(NTADR_A(6,26));
  
  //vram_adr(NTADR_A(2,3));
  //vram_write("    2 player mode", 18);
  //vram_adr(NTADR_A(6,26));
  
   vram_adr(NTADR_A(5,3));

  vram_write("\x1c\x1d\x1e\x1f or ""wasd"" to move!", 21);
  vram_adr(NTADR_A(20,24));
  
  
  

      
  
  vram_write("SCORE: ", 6);
  ppu_on_all();
  
 
  // figure out how to make this number change 
   //Draws and updates Scoreboard
 
  
   
  
  //vram_write( "0" , 1);
  
  
  
  // setup graphics
  // initialize actors with random values
  
  
  //Switch for either selected 1 or 2 player
  //START(Enter) is 1 Player
  //UP is 2 Players
  switch(choice)
  {
    case 1:
      for (i=0; i<NUM_ACTORS - 1; i++) {
    	 actor_x[i] = i*32+128;
   	 actor_y[i] = i*8+64;
   	 actor_dx[i] = 0;
    	 actor_dy[i] = 0;
  }break;
    case 2:
  	for (i=0; i<NUM_ACTORS; i++) {
    	 actor_x[i] = i*32+128;
   	 actor_y[i] = i*8+64;
   	 actor_dx[i] = 0;
    	 actor_dy[i] = 0;
  }break;
  }
  
  
  // loop forever (GAME LOOP)
  while (1) {
    
    // start with OAMid/sprite 0
    oam_id = 0;
    // set player 0/1 velocity based on controller
    for (i=0; i<2; i++) {
      // poll controller i (0-1)
      pad = pad_poll(i);
      // move actor[i] left/right
      if (pad&PAD_LEFT && actor_x[i]>0) actor_dx[i]=-2;
      else if (pad&PAD_RIGHT && actor_x[i]<240) actor_dx[i]=2;
      else actor_dx[i]=0;
      // move actor[i] up/down
      if (pad&PAD_UP && actor_y[i]>0) actor_dy[i]=-2;
      else if (pad&PAD_DOWN && actor_y[i]<212) actor_dy[i]=2;
      else actor_dy[i]=0;
    }
    // draw and move all actors
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 7;
      if (actor_dx[i] >= 0)
        runseq += 8;
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[runseq]);
      actor_x[i] += actor_dx[i];
      actor_y[i] += actor_dy[i];
    }
    
    
  //Draws and updates Scoreboard
    oam_id = oam_spr(206, 191, (score/10%10)+48, 1, oam_id);
    oam_id = oam_spr(213, 191, (score%10)+48, 1, oam_id);
  
    for(i = 0; i<10; i++)
     if(objs[i].sprite==20)
     oam_id = oam_spr(objs[i]._x, objs[i]._y, objs[i].sprite, 1, oam_id);
    else
      oam_id = oam_spr(objs[i]._x, objs[i]._y, objs[i].sprite, 1, oam_id);
      

    for(i=0;i<10;i++){
      if(rndint(1,2)==1)		//obj stays for random set of time
        objs[i].falling = true;
      
      if(objs[i].falling)		//Set object Fall speed 
      	objs[i]._dy = rndint(1,2);
        
      objs[i]._y += objs[i]._dy;	//Make obj Fall
      obj_collision(i);		// Check Collsion with Player
    }	
    // hide rest of sprites
    // if we haven't wrapped oam_id around to 0
    if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    ppu_wait_frame();
    
    
    
  }
}


void obj_collision(int o){
  if(objs[o]._y >= 210 || ((objs[o]._x >= actor_x[0]-4 && objs[o]._x <= actor_x[0]+8)&& (objs[o]._y >= actor_y[0]-2 && objs[o]._y <= actor_y[0]+4))) //hits floor or collision detected
      {
        if(objs[o]._y < 195){
          
          score += objs[o].points;
          objs[o]._y = 210;
        }
        objs[o].sprite=0; // erase obj for a bit
      }
  
  if(objs[o].sprite == 0 && objs[o]._y <= 160 && objs[o]._y >= 130 ){// Make obj reappear on random spot
       	objs[o]._x = rndint(20,230);
    	objs[o]._y = rndint(10,50);
        objs[o]._dy = 0;
    
    //set sprite before reappearing
    switch(o){
      case 0:
        objs[o].sprite=19;
        objs[o].falling=false;
        break;
      case 1:
        objs[o].sprite=20;
        objs[o].falling=false;
        break;
      case 2:
        objs[o].sprite=21;
        objs[o].falling=false;
        break;
      case 3:
        objs[o].sprite= 22;
        objs[o].falling=false;
        break;
         case 4:
        objs[o].sprite=23;
        objs[o].falling=false;
        break;
      case 5:
        objs[o].sprite= 24;
        objs[o].falling=false;
        break;
      default:
        break;
 	}
  }
  

}


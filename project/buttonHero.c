#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>

#define GREEN_LED BIT6

AbRect rect15 = {abRectGetBounds, abRectCheck, {15, 15}};

AbRectOutline fieldOutline = { // Where the bombs will be displayed
  abRectOutlineBounds, abRectOutlineCheck,
  {screenWidth/2 - 2, screenHeight/2 - 2}
};

Layer bomb1 = {
  (AbShape *)&circle15,
  {(screenWidth/2) + 10, (screenHeight/2) + 5},
  {0,0}, {0,0},
  COLOR_ORANGE,
  0,
};

Layer bomb2 = {
  (AbShape *)&circle15,
  {(screenWidth/2) + 10, (screenHeight/2) + 5},
  {0,0}, {0,0},
  COLOR_RED,
  &bomb1,
};

Layer bomb3 = {
  (AbShape *)&circle15,
  {(screenWidth/2) + 10, (screenHeight/2) + 5},
  {0,0}, {0,0},
  COLOR_BLACK,
  &bomb2,
};

Layer fieldLayer = {
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  &bomb3,
};

Layer bomb4 = {
  (AbShape *)&circle14,
  {(screenWidth/2)+10, (screenHeight/2+5},
   {0,0}, {0,0},
   COLOR_RED,
   &fieldLayer,
};

typedef strunt MovLayer_s{
  Layer *layer;
  Vec2 velocity;
  struct Mov_layer_s *next;
} MovLayer;

MovLayer m13 = {&bomb3, {1, 1}, 0};
MovLayer m11 = {&bomb1, {1, 2}, &m13};
MovLayer m10 = {&bomb0, {2, 1}, &m11};

movLayerDraw(MovLayer *movLayers, layer *layers){
  int row, col;
  MovLayer *movLayer;

  and_sr(~8); // disable interrupts
  for(movLayer = movLayers; movLayer; movLayer = movLayer->next){
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    1->pos = l->posNext;
  }
  or_sr(~8);

  for(movLayer = movLayers; movLayer; movLayer = movLayer->next){
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    col = bounds.topLeft.axes[0];
    for(row = bounds.topLeft.axes[1]; row <= bounds.botRightaxes[1]; row++){
      Vec2 pixelPos = {col, row};
      u_int color = bgColor;
      Layer *probeLayer;
      for(probeLayer = layers; probeLayer; probeLayer = probeLayer->next){
	if(abShapeCheck(probeLayer-<abShape, &probeLayer->pos, &pixelPos)){
	  color = probeLayer->color;
	  break;
	}
	lcd_writeColor(color);
      }// check all layers
    } // for row
  }
}

void mlAdvance(MovLayer *ml, Region *fence){
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for(;ml; ml = ml->next){
    vec2Add(&newPos, &ml->layer->posNext, &mlvelocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for(axis = 0; axis < 2; axis++){
      if((shapeBoundary.topLeft.axes[axis]  < fence->topLeft.axes[axis]) ||
	 (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])){
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }//outside of fence
    }// for axis
    ml->layer->posNext = newPos;
  }//for ml
}

u_int bgColor = COLOR_WHITE;
int redrawScreem = 1;

Region fieldFence;

void main(){
  P1DIR |= GREEN_LED;
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(1);

  shapeInit();
  layerInit(&bomb1);
  layerDraw(&bomb1);

  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();
  or_sr(0x8);

  for(;;){
    while(!redrawScreen){
      P1OUT &= ~GREEN_LED;
      or_sr(0x10);
    }
    P1OUT |= GREEN_LED;
    redrawScreen = 0;
    movLayerDraw(&m10, &bomb4);
  }
}

  void wdt_c_handler(){
    static short count = 0;
    P1OUT |= GREEN_LED;
    count++;
    if(count == 15){
      mlAdvance(&m10, &fieldFence);
      if(p2sw_read()){
	redrawScreen = 1;
      }
      count = 0;
    }
    P1OUT &= ~GREEN_LED;
  }


/* XBoard.C */

#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xboard.h"
#include "types.h"
#include "board.h"
#include "output.h"
#include "turtle.h"

/* Constants */

#define WINDOW_TITLE TURTLE_NAME
#define ICON_TITLE   "Turtle"

#define BOARD_COLOUR "Green3"
#define BOARD_FONT   "8x13"

/* Variables */

int XBoard = FALSE;
xboard board;
int SquareX, SquareY, Key;

static Display *display;
static Window window;
static ulong black, white, green;
static XFontStruct *font_struct;
static GC black_gc, white_gc, green_gc;

static int CornerX, CornerY;
static int SizeX = 43, SizeY = 43, MinSizeX = 7, MinSizeY = 7;
static int SpaceX = 1, SpaceY = 1;

static xboard view;

/* Prototypes */

static void CompSize    (int Width, int Height);
static void ClearXBoard (void);
static void DrawSquare  (int X, int Y, int Colour, const char *String);

/* Functions */

/* OpenXBoard() */

void OpenXBoard(void) {

   int screen;
   Window root;
   int width, height;
   XSetWindowAttributes attributes;
   XSizeHints size_hints;
   XWMHints wm_hints;
   Colormap colormap;
   XColor color;

   XBoard = FALSE;

   display = XOpenDisplay("");
   if (display == NULL) {
      Warning("Couldn't open display");
      return;
   }

   screen = DefaultScreen(display);
   root   = DefaultRootWindow(display);

   width  = 8 * SizeX + 16;
   height = 8 * SizeY + 16;

   attributes.backing_store = Always;

   window = XCreateWindow(display,root,0,0,width,height,0,CopyFromParent,CopyFromParent,CopyFromParent,CWBackingStore,&attributes);

   size_hints.width        = width;
   size_hints.height       = height;
   size_hints.min_width    = 16;
   size_hints.min_height   = 16;
   size_hints.width_inc    = 8;
   size_hints.height_inc   = 8;
   size_hints.min_aspect.x = 1;
   size_hints.min_aspect.y = 1;
   size_hints.max_aspect.x = 1;
   size_hints.max_aspect.y = 1;
   size_hints.flags        = PSize | PMinSize | PResizeInc | PAspect;

   XSetStandardProperties(display,window,WINDOW_TITLE,ICON_TITLE,None,ArgV,ArgC,&size_hints);

   wm_hints.input         = True;
   wm_hints.initial_state = NormalState;
   wm_hints.flags         = InputHint | StateHint;

   XSetWMHints(display,window,&wm_hints);

   font_struct = XLoadQueryFont(display,BOARD_FONT);

   black = BlackPixel(display,screen);
   white = WhitePixel(display,screen);

   black_gc = XCreateGC(display,window,0,NULL);
   XSetForeground(display,black_gc,black);
   XSetFont(display,black_gc,font_struct->fid);

   white_gc = XCreateGC(display,window,0,NULL);
   XSetForeground(display,white_gc,white);
   XSetFont(display,white_gc,font_struct->fid);

   colormap = DefaultColormap(display,screen);
   XParseColor(display,colormap,BOARD_COLOUR,&color);
   XAllocColor(display,colormap,&color);
   green = color.pixel;

   green_gc = XCreateGC(display,window,0,NULL);
   XSetForeground(display,green_gc,green);
   XSetFont(display,green_gc,font_struct->fid);

   XSelectInput(display,window,ExposureMask|StructureNotifyMask|ButtonPressMask|KeyPressMask);
   XMapRaised(display,window);

   ClearXBoard();

   XBoard = TRUE;
}

/* CloseXBoard() */

void CloseXBoard(void) {

   if (XBoard) {
      XDestroyWindow(display,window);
      XFreeGC(display,black_gc);
      XFreeGC(display,white_gc);
      XFreeGC(display,green_gc);
      XFreeFont(display,font_struct);
      XCloseDisplay(display);
   }
}

/* HandleXEvent() */

int HandleXEvent(void) {

   XEvent event;
   KeySym key;
   char Buffer[16];

   while (XBoard && XEventsQueued(display,QueuedAfterReading) != 0) {

      XNextEvent(display,&event);

      switch(event.type) {

      case Expose :

         if (event.xexpose.count == 0) {
            ClearXBoard();
            DispXBoard();
         }
         break;

      case ConfigureNotify :

         CompSize(event.xconfigure.width,event.xconfigure.height);
         break;

      case DestroyNotify :

         XBoard = FALSE;
         break;

      case MappingNotify :

         XRefreshKeyboardMapping(&event.xmapping);
         break;

      case ButtonPress :

         SquareX = event.xbutton.x - CornerX - 1;
         SquareY = event.xbutton.y - CornerY - 1;

         if (SquareX >= 0 && SquareY >= 0
          && SquareX < 8 * SizeX + 14 && SquareY < 8 * SizeY + 14
	  && SquareX % (SizeX + 2) < SizeX && SquareY % (SizeY + 2) < SizeY) {
            SquareX /= SizeX + 2;
            SquareY /= SizeY + 2;
            return XSQUARE;
         }

         break;

      case KeyPress :

         if (XLookupString(&event.xkey,Buffer,16,&key,NULL) == 1) {
            Key = Buffer[0];
            return XKEY;
         }
         break;
      }
   }

   return XNONE;
}

/* DispXBoard() */

void DispXBoard(void) {

   int X, Y;

   if (! XBoard) return;

   if (view.Colour != board.Colour) {
      for (Y = 0; Y < 8; Y++) {
         for (X = 0; X < 8; X++) {
            if (view.Square[Y][X].Colour == EMPTY && board.Square[Y][X].Colour == EMPTY) {
               strcpy(view.Square[Y][X].String,"*BAD*");
            }
         }
      }
   }
   view.Colour = board.Colour;

   for (Y = 0; Y < 8; Y++) {
      for (X = 0; X < 8; X++) {
	 if (view.Square[Y][X].Colour != board.Square[Y][X].Colour
	  || strcmp(view.Square[Y][X].String,board.Square[Y][X].String) != 0) {
            DrawSquare(X,Y,board.Square[Y][X].Colour,board.Square[Y][X].String);
            view.Square[Y][X].Colour = board.Square[Y][X].Colour;
            strcpy(view.Square[Y][X].String,board.Square[Y][X].String);
         }
      }
   }

   XFlush(display);
}

/* CompSize() */

static void CompSize(int Width, int Height) {

   int NewWidth, NewHeight;

   SizeX = (Width  - 16) / 8;
   SizeY = (Height - 16) / 8;

   if (SizeX > SizeY) {
      SizeX = SizeY;
   } else if (SizeX < SizeY) {
      SizeY = SizeX;
   }

   if (SizeX < MinSizeX) SizeX = MinSizeX;
   if (SizeY < MinSizeY) SizeY = MinSizeY;

   NewWidth  = 8 * SizeX + 16;
   NewHeight = 8 * SizeY + 16;

   CornerX = (Width  - 8 * SizeX - 16) / 2;
   CornerY = (Height - 8 * SizeY - 16) / 2;

   if (Width != NewWidth || Height != NewHeight) {
      XResizeWindow(display,window,NewWidth,NewHeight);
   }
}

/* ClearXBoard() */

static void ClearXBoard(void) {

   int I, X, Y;

   XClearWindow(display,window);

   XFillRectangle(display,window,green_gc,CornerX+1,CornerY+1,8*SizeX+14,8*SizeY+14);

   for (I = 1; I <= 8; I++) {
      XDrawLine(display,window,black_gc,CornerX+1,CornerY+I*(SizeY+2)-1,CornerX+8*SizeX+15,CornerY+I*(SizeY+2)-1);
      XDrawLine(display,window,black_gc,CornerX+I*(SizeX+2)-1,CornerY+1,CornerX+I*(SizeX+2)-1,CornerY+8*SizeY+15);
   }

   for (I = 0; I <= 7; I++) {
      XDrawLine(display,window,white_gc,CornerX,CornerY+I*(SizeY+2),CornerX+8*SizeX+15,CornerY+I*(SizeY+2));
      XDrawLine(display,window,white_gc,CornerX+I*(SizeX+2),CornerY,CornerX+I*(SizeX+2),CornerY+8*SizeY+15);
   }

   XFlush(display);

   view.Colour = EMPTY;
   for (Y = 0; Y < 8; Y++) {
      for (X = 0; X < 8; X++) {
         view.Square[Y][X].Colour = EMPTY;
         strcpy(view.Square[Y][X].String,"");
      }
   }
}

/* DrawSquare() */

static void DrawSquare(int X, int Y, int Colour, const char *String) {

   int Len;
   XCharStruct char_struct;
   int Direction, High, Low;
   int Cx, Cy;

   if (String == NULL) String = "";
   Len = strlen(String);

   switch (Colour) {

   case EMPTY :

      XFillRectangle(display,window,green_gc,CornerX+X*(SizeX+2)+1,CornerY+Y*(SizeY+2)+1,SizeX,SizeY);

      if (Len != 0) {

         XTextExtents(font_struct,String,Len,&Direction,&High,&Low,&char_struct);
         Cx = SizeX / 2 - (char_struct.rbearing - char_struct.lbearing) / 2;
         Cy = SizeY / 2 + (char_struct.ascent   - char_struct.descent)  / 2;

         if (view.Colour == WHITE) {
            XDrawString(display,window,white_gc,CornerX+X*(SizeX+2)+Cx+1,CornerY+Y*(SizeY+2)+Cy+1,String,Len);
         } else {
            XDrawString(display,window,black_gc,CornerX+X*(SizeX+2)+Cx+1,CornerY+Y*(SizeY+2)+Cy+1,String,Len);
         }
      }

      break;

   case BLACK :

      XFillArc(display,window,black_gc,CornerX+X*(SizeX+2)+SpaceX+1,CornerY+Y*(SizeY+2)+SpaceY+1,SizeX-2*SpaceX-1,SizeY-2*SpaceY-1,0,360*64);
      XDrawArc(display,window,white_gc,CornerX+X*(SizeX+2)+SpaceX+1,CornerY+Y*(SizeY+2)+SpaceY+1,SizeX-2*SpaceX-1,SizeY-2*SpaceY-1,0,360*64);

      if (Len != 0) {

         XTextExtents(font_struct,String,Len,&Direction,&High,&Low,&char_struct);
         Cx = SizeX / 2 - (char_struct.rbearing - char_struct.lbearing) / 2;
         Cy = SizeY / 2 + (char_struct.ascent   - char_struct.descent)  / 2;

         XDrawString(display,window,white_gc,CornerX+X*(SizeX+2)+Cx+1,CornerY+Y*(SizeY+2)+Cy+1,String,Len);
      }

      break;

   case WHITE :

      XFillArc(display,window,white_gc,CornerX+X*(SizeX+2)+SpaceX+1,CornerY+Y*(SizeY+2)+SpaceY+1,SizeX-2*SpaceX-1,SizeY-2*SpaceY-1,0,360*64);
      XDrawArc(display,window,black_gc,CornerX+X*(SizeX+2)+SpaceX+1,CornerY+Y*(SizeY+2)+SpaceY+1,SizeX-2*SpaceX-1,SizeY-2*SpaceY-1,0,360*64);

      if (Len != 0) {

         XTextExtents(font_struct,String,Len,&Direction,&High,&Low,&char_struct);
         Cx = SizeX / 2 - (char_struct.rbearing - char_struct.lbearing) / 2;
         Cy = SizeY / 2 + (char_struct.ascent   - char_struct.descent)  / 2;

         XDrawString(display,window,black_gc,CornerX+X*(SizeX+2)+Cx+1,CornerY+Y*(SizeY+2)+Cy+1,String,Len);
      }

      break;
   }
}

/* End of XBoard.C */


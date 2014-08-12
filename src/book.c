
/* Book.C */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "book.h"
#include "types.h"
#include "board.h"
#include "eval.h"
#include "game.h"
#include "loop.h"
#include "master.h"
#include "output.h"
#include "symmetry.h"

/* Constants */

#define BOOK_DIR     "data/book/"

#define MIN_EMPTIES  16
#define SMOOTH       18

#define DISKDIFF_MIN (-64/2-1)
#define DISKDIFF_MAX (+64/2+1)
#define EVAL_MIN     (-INF-1)
#define EVAL_MAX     (+INF+1)

/* Types */

typedef struct Node NODE;

struct Node {
   uchar IsLink;
   uchar Done;
   uchar Move;
   uchar Symmetry;
   uchar Depth;
   schar DiskDiff;
   short Eval;
   short Value;
   short IosValue;
   NODE *Son;
   NODE *Brother;
};

typedef struct {
   NODE *Node;
   int   Move[GAME_MAX_LENGTH+1];
   int   Sym;
} PATH;

/* Variables */

int     UseBook;
char   *BookFileName;
int     DrawValue;
int     SwitchValue;
int     LearnGame;
double  LearningTime;
char   *LearnLine;

static NODE *Root;

static int Smooth[61];

static int BookNodeNb;
static int BookGameNb;
static int BookLeafNb;
static int BookDrawValue;

/* Prototypes */

static void  LoadSmooth   (void);

static void  AddLines     (void);
static void  AddLine      (const char *Line);
static void  AddPath      (const PATH *Path, int Learn);

static void  UpdateBook   (void);
static void  UpdateTree   (NODE *Node, int Colour, int MoveNo);

static void  NegaMaxBook  (void);
static void  NegaMaxTree  (NODE *Node);

static int   CountBook    (void);
static int   CountTree    (NODE *Node, BOARD *Board);
static void  NoteBook     (void);
static void  NoteTree     (NODE *Node, BOARD *Board);

static void  LoadBook     (void);
static void  LoadTree     (FILE *File, NODE *Node);
static void  MergeTree    (FILE *File, NODE *Node, BOARD *Board);
static void  SaveBook     (void);
static void  SaveTree     (FILE *File, const NODE *Node);

static void  CreateLinks  (void);
static void  CreateLinks2 (NODE *Node, BOARD *Board);

static void  ExpandBook   (void);

static void  CheckBook    (void);
static void  CheckTree    (NODE *Node, int Sym, BOARD *Board);

static NODE *NewNode      (void);
static NODE *NewSon       (NODE *Node, int Sym, const BOARD *Board, int Move);
static NODE *BestSon      (const NODE *Node, int Sym, const int Forbidden[]);

static NODE *NodeSon      (const NODE *Node);
static int   IsNode       (const NODE *Node);
static int   IsLeaf       (const NODE *Node);
static int   IsEmptyNode  (const NODE *Node, const BOARD *Board);

static void  StoreBest    (NODE *Node, const BEST_MOVE *Best);

static int   BoardToPath  (const BOARD *Board, PATH *Path);
static NODE *BoardToPath2 (NODE *Node, BOARD *Board, int Move[], const BOARD *Goal);

static int   NodeToPath   (const NODE *Node, PATH *Path);
static NODE *NodeToPath2  (NODE *Node, int Move[], const NODE *Goal);

static void  AsciiToPath  (const char *Line, PATH *Path);
static int   GameToPath   (const GAME *Game, PATH *Path);

static void  PathToAscii  (const PATH *Path);

/* Functions */

/* InitBook() */

void InitBook(void) {

   Root = NewNode();

   if (UseBook) {

      BookDrawValue = DrawValue;

      LoadBook();

      printf("[ \"%s\" : %d bytes, %d games, %d leaves ] ",BookFileName,BookNodeNb*sizeof(NODE),BookGameNb,BookLeafNb);
      fflush(stdout);

      LoadSmooth();
      AddLines();
      NegaMaxBook();
      CheckBook();
      SaveBook();

   } else {

      printf("[ No book ] ");
      fflush(stdout);
   }
}

/* LoadSmooth() */

static void LoadSmooth(void) {

   FILE *File;
   int M;
   double S;

   for (M = 0; M <= 60; M++) Smooth[M] = SMOOTH;

   File = fopen(BOOK_DIR "smooth","r");
   if (File == NULL) {
      Error("Couldn't open smooth file \"%s\" for reading",BOOK_DIR "smooth");
      return;
   }

   while (fscanf(File,"%d %lf",&M,&S) != EOF) Smooth[M] = Round(S);

   fclose(File);
}

/* SetDrawValue() */

void SetDrawValue(int DrawValue) {

   if (BookDrawValue != DrawValue) {
      BookDrawValue = DrawValue;
      NegaMaxBook();
   }
}

/* LearnBook() */

void LearnBook(void) {

   printf("%d positions to learn...\n",CountBook());

   while (TRUE) {
      NoteBook();
      NegaMaxBook();
      CheckBook();
      SaveBook();
      ExpandBook();
      if (SwitchValue) BookDrawValue = -BookDrawValue;
   }
}

/* BookMove() */

int BookMove(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   PATH    Path[1];
   NODE   *Node;
   double  Eval;
   int    *PV;

   if (EmptyNb(Board) < MIN_EMPTIES || ! BoardToPath(Board,Path)) return FALSE;

   Node = BestSon(Path->Node,Path->Sym,Forbidden);
   if (Node == NULL || Node->Value < -MID_INF) return FALSE;

   Best->Move  = Symmetry[Path->Sym][Node->Move];
   Best->Depth = Node->Depth;

   Eval = BoundedEval(Node->Value);

   if (Node->DiskDiff == DISKDIFF_MIN) {
      Best->IosValue = -Eval;
      sprintf(Best->StringValue,"%.0f%%",100.0*DoubleEval(Node->Value));
   } else {
      Best->IosValue = (double) (2 * Node->DiskDiff);
      if (Node->DiskDiff >= 0) {
         Best->IosValue += Eval;
      } else {
         Best->IosValue -= Eval;
      }
      sprintf(Best->StringValue,"%.0f%%/%+d",100.0*DoubleEval(Node->Value),Node->DiskDiff);
   }

   PV    = Best->PV;
   *PV++ = Symmetry[Path->Sym][Node->Move];

   while (Node->Son != NULL) {

      Node = BestSon(Node,Path->Sym,NULL);
      if (Node == NULL) FatalError("Node == NULL in BookMove()");

      *PV++ = Symmetry[Path->Sym][Node->Move];

      if (Node->IsLink) {
         Path->Sym ^= Node->Symmetry;
         Node = Node->Son;
      }
   }

   *PV = NO_MOVE;

   Best->Time = Duration(Search->Start,CurrentTime());

   return TRUE;
}

/* AddLines() */

static void AddLines(void) {

   FILE *File;
   char Line[256];

   AddLine("F5 D6 C3 D3 C4 F4 F6");
   AddLine("F5 F6 E6 F4 E3 D6");

   AddLine("D3");
   AddLine("C4");
   AddLine("E6");
   AddLine("F5 F6 E6 D6");
   AddLine("F5 D6 C4 D3 C3");
   AddLine("F5 F4 E3 F6 E6");
   AddLine("F5 F4 E3 D6 E6 F6");
   AddLine("F5 D6 C3 F4 F6 D3 C4");

   File = fopen(BOOK_DIR "lines","r");
   if (File != NULL) {
      while (fgets(Line,256,File) != NULL) AddLine(Line);
      fclose(File);
      remove(BOOK_DIR "lines");
   }
}

/* AddLine() */

static void AddLine(const char *Line) {

   PATH Path[1];

   AsciiToPath(Line,Path);
   AddPath(Path,TRUE);
}

/* AddGame() */

void AddGame(const GAME *Game) {

   PATH Path[1];

   if (GameToPath(Game,Path)) {

      AddPath(Path,TRUE);

      NegaMaxBook();
      CheckBook();
      SaveBook();

      if (LearnGame) {
         NoteBook();
         NegaMaxBook();
         CheckBook();
         SaveBook();
      }
   }
}

/* Normalize() */

void Normalize(int *Binary) {

   int   I, Sym;
   PATH  Path[1];
   NODE *Node, *Link;

   for (I = 0; I < 7 && Binary[I] != NO_MOVE; I++) Path->Move[I] = Binary[I];
   Path->Move[I] = NO_MOVE;
   AddPath(Path,FALSE);

   Link = NULL;
   Node = Root;
   Sym  = ID_SYM;

   for (I = 0; Binary[I] != NO_MOVE; I++) {
      for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
         if (Symmetry[Sym][Node->Move] == Binary[I]) break;
      }
      if (Node == NULL) break;
      if (Node->IsLink) {
         Sym ^= Node->Symmetry;
         Node = Node->Son;
         Link = Node;
      }
   }

   if (Link != NULL) {
      NodeToPath(Link,Path);
      for (I = 0; Path->Move[I] != NO_MOVE; I++) Binary[I] = Path->Move[I];
      for (; Binary[I] != NO_MOVE; I++) Binary[I] = Symmetry[Sym][Binary[I]];
   }
}

/* AddPath() */

static void AddPath(const PATH *Path, int Learn) {

   const int *PathMove;
   int        Sym;
   NODE      *Node;
   BOARD      Board[1];
   BEST_MOVE  Best[1];

   Node = Root;
   ClearBoard(Board);
   Sym = ID_SYM;

   for (PathMove = Path->Move; *PathMove != NO_MOVE; PathMove++) {
      if (! IsLegalMove(Board,*PathMove)) FatalError("Illegal move in AddPath()");
      if (EmptyNb(Board) == MIN_EMPTIES && *PathMove != PASS) break;
      DoMove(Board,*PathMove);
      Node = NewSon(Node,Sym,Board,*PathMove);
      if (Node->IsLink) {
         Sym ^= Node->Symmetry;
         Node = Node->Son;
      }
   }

   if (Node->Son == NULL) {

      if (IsFinished(Board)) {

         Node->DiskDiff = -DiskDiff(Board) / 2;
         Node->Depth    = SOLVE_DEPTH;
         Node->Eval     = DIFFNOTE(2*Node->DiskDiff);
         Node->Value    = Node->Eval;

         if (Node->DiskDiff == 0) {
            Node->Value = (Board->Colour > 0) ? -BookDrawValue : +BookDrawValue;
         }

      } else if (Learn) {

         InitThinkBK(Board,NULL,Best);
         ThinkBook();
         if (Best->Move == NO_MOVE) FatalError("NO_MOVE in AddPath()");

         DoMove(Board,Best->Move);
         Node = NewSon(Node,Sym,Board,Best->Move);

         StoreBest(Node,Best);
      }
   }
}

/* UpdateBook() */

static void UpdateBook(void) {

   UpdateTree(Root,BLACK,0);
}

/* UpdateTree() */

static void UpdateTree(NODE *Node, int Colour, int MoveNo) {

   if (! Node->IsLink) {
      if (Node->Son == NULL) {
	 Node->Value    = Node->Eval;
	 Node->DiskDiff = DISKDIFF_MIN;
	 if (Node->Depth == SOLVE_DEPTH) {
	    Node->DiskDiff = UNDIFFNOTE(Node->Eval) / 2;
	    if (Node->DiskDiff == 0) {
	       Node->Value = (Colour > 0) ? -BookDrawValue : +BookDrawValue;
	    }
	 } else if (Node->Depth < SELECTIVE_DEPTH && Node->Eval >= -MID_INF && Node->Eval <= +MID_INF) {
            if (MoveNo <= 0 || MoveNo >= 60) FatalError("MoveNo = %d in UpdateTree()",MoveNo);
	    Node->Value += (Node->Depth % 2 == 0) ? +Smooth[MoveNo] : -Smooth[MoveNo];
	 }
      } else {
         for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
 	    if (Node->Move == PASS) {
               UpdateTree(Node,-Colour,MoveNo);
            } else {
               UpdateTree(Node,-Colour,MoveNo+1);
            }
         }
      }
   }
}

/* NegaMaxBook() */

static void NegaMaxBook(void) {

   UpdateBook();
   NegaMaxTree(Root);
}

/* NegaMaxTree() */

static void NegaMaxTree(NODE *Node) {

   int BestDepth, BestValue, BestDiskDiff;
   NODE *Son;

   BestDepth    = 0;
   BestValue    = EVAL_MIN;
   BestDiskDiff = DISKDIFF_MIN;

   for (Son = Node->Son; Son != NULL; Son = Son->Brother) {
      if (! Son->IsLink) {
         if (IsNode(Son)) NegaMaxTree(Son);
      } else {
         if (IsNode(Son)) NegaMaxTree(Son->Son);
         Son->Depth    = Son->Son->Depth;
         Son->Value    = Son->Son->Value;
         Son->DiskDiff = Son->Son->DiskDiff;
      }
      if (Son->DiskDiff > BestDiskDiff) BestDiskDiff = Son->DiskDiff;
   }

   Son = BestSon(Node,ID_SYM,NULL);

   if (IsNode(Son)) {
      BestDepth = Son->Depth;
   } else {
      BestDepth = 0;
   }
   BestValue = Son->Value;

   Node->Depth    = BestDepth + 1;
   Node->Value    = (BestValue    == EVAL_MIN)     ? BestValue    : -BestValue;
   Node->DiskDiff = (BestDiskDiff == DISKDIFF_MIN) ? BestDiskDiff : -BestDiskDiff;
}

/* CountBook() */

static int CountBook(void) {

   BOARD Board[1];

   ClearBoard(Board);

   return CountTree(Root,Board);
}

/* CountTree() */

static int CountTree(NODE *Node, BOARD *Board) {

   int Count;

   Count = 0;

   if (IsEmptyNode(Node,Board)) Count++;

   for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
      if (! Node->IsLink && IsNode(Node)) {
         DoMove(Board,Node->Move);
         Count += CountTree(Node,Board);
         UndoMove(Board,Node->Move);
      }
   }

   return Count;
}

/* NoteBook() */

static void NoteBook(void) {

   BOARD Board[1];

   ClearBoard(Board);
   NoteTree(Root,Board);
}

/* NoteTree() */

static void NoteTree(NODE *Node, BOARD *Board) {

   int I;
   NODE *Son;
   static int Forbidden[SQUARE_NB];
   static BEST_MOVE Best[1];

   if (IsEmptyNode(Node,Board)) {

      do {

         for (I = 0; I < SQUARE_NB; I++) Forbidden[I] = FALSE;
         for (Son = Node->Son; Son != NULL; Son = Son->Brother) {
            Forbidden[Son->Move] = TRUE;
         }

         DispBoard2(Board,Forbidden);
         InitThinkBK(Board,Forbidden,Best);
         Think();
         if (Best->Move == NO_MOVE) break;

         DoMove(Board,Best->Move);
         Son = NewSon(Node,ID_SYM,Board,Best->Move);
         UndoMove(Board,Best->Move);

         StoreBest(Son,Best);

      } while (Son->IsLink);

      NegaMaxBook();
      CheckBook();
      SaveBook();
   }

   for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
      if (! Node->IsLink && IsNode(Node)) {
         DoMove(Board,Node->Move);
         NoteTree(Node,Board);
         UndoMove(Board,Node->Move);
      }
   }
}

/* LoadBook() */

static void LoadBook(void) {

   char FileName[256];
   FILE *BookFile;
   BOARD Board[1];

   BookNodeNb = 0;
   BookGameNb = 0;
   BookLeafNb = 0;

   sprintf(FileName,BOOK_DIR "%s",BookFileName);
   BookFile = fopen(FileName,"rb");
   if (BookFile == NULL) {
      Error("Couldn't open book file \"%s\" for reading",FileName);
      return;
   }

   ClearBoard(Board);

   if (Root->Son == NULL) {
      LoadTree(BookFile,Root);
      CreateLinks();
   } else {
      MergeTree(BookFile,Root,Board);
   }

   fclose(BookFile);
}

/* LoadTree() */

static void LoadTree(FILE *File, NODE *Node) {

   int SonNb, Data;

   SonNb = fgetc(File);

   Node->Son = NewNode();
   Node      = Node->Son;

   do {

      Data = fgetc(File);
      Node->Move = Data / 3;

      switch (Data % 3) {

         case 0 :  LoadTree(File,Node);
                   break;

         case 1 :  Node->Depth =  fgetc(File);
                   Node->Eval  =  fgetc(File) << 8;
                   Node->Eval  += fgetc(File) + EVAL_MIN;
                   if (Node->Depth == SOLVE_DEPTH) {
                      BookGameNb++;
                   } else {
                      BookLeafNb++;
                   }
                   break;

         default : Node->IsLink = TRUE;
                   break;
      }

      if (SonNb > 1) {
         Node->Brother = NewNode();
         Node          = Node->Brother;
      }

   } while (--SonNb != 0);
}

/* MergeTree() */

static void MergeTree(FILE *File, NODE *Node, BOARD *Board) {

   int SonNb, Data, Type, Move;

   SonNb = fgetc(File);

   do {

      Data = fgetc(File);
      Move = Data / 3;
      Type = Data % 3;

      if (Move == NO_MOVE)           FatalError("NO_MOVE in MergeTree()");
      if (! IsLegalMove(Board,Move)) FatalError("Illegal move in MergeTree()");
      DoMove(Board,Move);
      Node = NewSon(Node,ID_SYM,Board,Move);

      if (Type == 0) {
         MergeTree(File,Node,Board);
      } else if (Type == 1) {
         Node->Depth =  fgetc(File);
         Node->Eval  =  fgetc(File) << 8;
         Node->Eval  += fgetc(File) + EVAL_MIN;
      }

      UndoMove(Board,Move);

   } while (--SonNb != 0);
}

/* SaveBook() */

static void SaveBook(void) {

   char  FileName[256];
   FILE *BookFile;

   if (Root->Son == NULL) {
      Warning("Tried to save an empty book");
      return;
   }

   sprintf(FileName,BOOK_DIR "%s",BookFileName);
   BookFile = fopen(FileName,"wb");
   if (BookFile == NULL) {
      Error("Couldn't open book file \"%s\" for writing",BookFileName);
      return;
   }

   SaveTree(BookFile,Root);

   fclose(BookFile);
}

/* SaveTree() */

static void SaveTree(FILE *File, const NODE *Node) {

   int SonNb;
   const NODE *Son;

   SonNb = 0;
   for (Son = Node->Son; Son != NULL; Son = Son->Brother) SonNb++;
   fputc(SonNb,File);

   for (Son = Node->Son; Son != NULL; Son = Son->Brother) {
      if (Son->Son == NULL) {
         fputc(3*Son->Move+1,File);
         fputc(Son->Depth,File);
         fputc((Son->Eval-EVAL_MIN)>>8,File);
         fputc((Son->Eval-EVAL_MIN)&0xFF,File);
      } else if (Son->IsLink) {
         fputc(3*Son->Move+2,File);
      } else {
         fputc(3*Son->Move,File);
         SaveTree(File,Son);
      }
   }
}

/* CreateLinks() */

static void CreateLinks(void) {

   BOARD Board[1];

   if (Root == NULL) return;

   ClearBoard(Board);
   CreateLinks2(Root,Board);
}

/* CreateLinks2() */

static void CreateLinks2(NODE *Node, BOARD *Board) {

   if (Node->IsLink) {

      PATH LinkPath[1];

      if (BoardToPath(Board,LinkPath)) {
         Node->Symmetry = LinkPath->Sym;
         Node->Son      = LinkPath->Node;
      } else {
         FatalError("Link not found in CreateLinks2()");
      }

   } else {

      for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
         if (Node->Son != NULL || Node->IsLink) {
            DoMove(Board,Node->Move);
            CreateLinks2(Node,Board);
            UndoMove(Board,Node->Move);
         }
      }
   }
}

/* ExpandBook() */

static void ExpandBook(void) {

   PATH Path[1];
   int *PathMove;
   int Sym;
   NODE *Node;
   BOARD Board[1];
   BEST_MOVE Best[1];

   if (Root == NULL) return;

   AsciiToPath(LearnLine,Path);

   Node = Root;
   ClearBoard(Board);
   Sym = ID_SYM;

   for (PathMove = Path->Move; *PathMove != NO_MOVE; PathMove++) {
      if (! IsLegalMove(Board,*PathMove)) FatalError("Illegal move in ExpandBook()");
      if (EmptyNb(Board) == MIN_EMPTIES && *PathMove != PASS) break;
      DoMove(Board,*PathMove);
      Node = NewSon(Node,Sym,Board,*PathMove);
      if (Node->IsLink) {
         Sym ^= Node->Symmetry;
         Node = Node->Son;
      }
   }

   while (Node->Son != NULL) {

      Node = BestSon(Node,Sym,NULL);
      if (Node == NULL) FatalError("Node == NULL in ExpandBook()");

      DoMove(Board,Symmetry[Sym][Node->Move]);

      if (Node->IsLink) {
         Sym ^= Node->Symmetry;
         Node = Node->Son;
      }
   }

   if (IsFinished(Board) || EmptyNb(Board) <= MIN_EMPTIES) {
      FatalError("Finished board in ExpandBook()");
   }

   InitThinkBK(Board,NULL,Best);
   ThinkBook();
   if (Best->Move == NO_MOVE) FatalError("NO_MOVE in ExpandBook()");

   DoMove(Board,Best->Move);
   Node = NewSon(Node,ID_SYM,Board,Best->Move);

   StoreBest(Node,Best);
}

/* CheckBook() */

static void CheckBook(void) {

   BOARD Board[1];

   ClearBoard(Board);
   CheckTree(Root,ID_SYM,Board);
}

/* CheckTree() */

static void CheckTree(NODE *Node, int Sym, BOARD *Board) {

/*
   BEST_MOVE Best[1];
*/

   if (Node == NULL) FatalError("NULL node in CheckTree()");

/*
   if (EmptyNb(Board) == 16 && ! Node->IsLink && Node->Son != NULL && Node->Son->Move != PASS) {

      DispBoard(Board);

      printf("%s\n",SquareString[Node->Son->Move]);

      Node->Depth    = 0;
      Node->Eval     = EVAL_MIN;
      Node->Value    = Node->Eval;
      Node->DiskDiff = DISKDIFF_MIN;
      Node->Son      = NULL;

      InitThinkBK(Board,NULL,Best);
      ThinkBook();
      if (Best->Move == NO_MOVE) FatalError("NO_MOVE in CheckTree()");

      DoMove(Board,Best->Move);
      Node = NewSon(Node,Sym,Board,Best->Move);
      UndoMove(Board,Best->Move);

      StoreBest(Node,Best);
   }
*/

   if (IsFinished(Board)) {
      if (Node->Son != NULL) FatalError("Finished node in CheckTree()");
      if (Node->Symmetry != ID_SYM) {
         FatalError("Finished symmetry leaf in CheckTree()");
      }
      if (Node->IsLink) FatalError("Finished link in CheckTree()");
      if (Node->Eval != DIFFNOTE(-DiskDiff(Board))) {
         Error("Incorrect finished eval in CheckTree()");
         Node->Eval = DIFFNOTE(-DiskDiff(Board));
      }
      if (Node->Depth != SOLVE_DEPTH) {
         Error("Incorrect finished depth in CheckTree()");
         Node->Depth = SOLVE_DEPTH;
      }
      if (Node->DiskDiff != -DiskDiff(Board) / 2) {
         Error("Incorrect finished diskdiff in CheckTree()");
         Node->DiskDiff = -DiskDiff(Board) / 2;
      }
   }

   if (Node->Son == NULL) {
      if (Node->IsLink) FatalError("Link leaf in CheckTree()");
      if (Node->Symmetry != ID_SYM) FatalError("Symmetry leaf in CheckTree()");
   } else if (! Node->IsLink) {
      if (Node->Symmetry != ID_SYM) FatalError("Symmetry node in CheckTree()");
   }

   if (Node->IsLink) {
      CheckTree(Node->Son,Sym^Node->Symmetry,Board);
   } else {
      for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
         if (Node->Move == NO_MOVE) {
            DispBoard(Board);
            FatalError("NO_MOVE in CheckTree()");
         }
         if (! IsLegalMove(Board,Symmetry[Sym][Node->Move])) {
            DispBoard(Board);
            FatalError("Illegal move : %s in CheckTree()",SquareString[Symmetry[Sym][Node->Move]]);
         }
         DoMove(Board,Symmetry[Sym][Node->Move]);
         CheckTree(Node,Sym,Board);
         UndoMove(Board,Symmetry[Sym][Node->Move]);
      }
   }
}

/* NewNode() */

static NODE *NewNode(void) {

   NODE *Node;

   Node = malloc(sizeof(NODE));
   if (Node == NULL) FatalError("Not enough memory for book node");

   BookNodeNb++;

   Node->IsLink   = FALSE;
   Node->Symmetry = ID_SYM;
   Node->Move     = NO_MOVE;
   Node->Depth    = 0;
   Node->Eval     = EVAL_MIN;
   Node->Value    = EVAL_MIN;
   Node->IosValue = 0;
   Node->DiskDiff = DISKDIFF_MIN;
   Node->Son      = NULL;
   Node->Brother  = NULL;

   return Node;
}

/* NewSon() */

static NODE *NewSon(NODE *Node, int Sym, const BOARD *Board, int Move) {

   PATH Path[1];

   Move = Symmetry[Sym][Move];

   if (Node->Son == NULL) {
      BoardToPath(Board,Path);
      Node->Son = NewNode();
      Node      = Node->Son;
   } else { /* Node->Son != NULL */
      for (Node = Node->Son; Node->Move != Move && Node->Brother != NULL; Node = Node->Brother)
         ;
      if (Node->Move == Move) return Node;
      BoardToPath(Board,Path);
      Node->Brother = NewNode();
      Node          = Node->Brother;
   }

   Node->Move = Move;

   if (Path->Node != NULL) {
      Node->IsLink   = TRUE;
      Node->Symmetry = Path->Sym ^ Sym;
      Node->Son      = Path->Node;
   }

   return Node;
}

/* BestSon() */

static NODE *BestSon(const NODE *Node, int Sym, const int Forbidden[]) {

   NODE *BestSon, *Son;
   int BestValue, BestDiskDiff, BestDepth, Depth;

   BestSon      = NULL;
   BestValue    = EVAL_MIN;
   BestDiskDiff = DISKDIFF_MIN;
   BestDepth    = 255;

   for (Son = Node->Son; Son != NULL; Son = Son->Brother) {
      Depth = (IsNode(Son)) ? Son->Depth : -Son->Depth;
      if ((Forbidden == NULL || ! Forbidden[Symmetry[Sym][Son->Move]])
       && (Son->Value    >  BestValue
       || (Son->Value    == BestValue
       && (Depth         <  BestDepth
       || (Depth         == BestDepth
       && (Son->DiskDiff >  BestDiskDiff
       || (Son->DiskDiff == BestDiskDiff
       && (Son->Symmetry == Sym)))))))) {
         BestSon      = Son;
         BestValue    = Son->Value;
         BestDepth    = Depth;
         BestDiskDiff = Son->DiskDiff;
      }
   }

   return BestSon;
}

/* NodeSon() */

static NODE *NodeSon(const NODE *Node) {

   NODE *Son;

   Son = Node->Son;
   if (Node->IsLink) Son = Son->Son;

   return Son;
}

/* IsNode() */

static int IsNode(const NODE *Node) {

   return NodeSon(Node) != NULL;
}

/* IsLeaf() */

static int IsLeaf(const NODE *Node) {

   return NodeSon(Node) == NULL;
}

/* IsEmptyNode() */

static int IsEmptyNode(const NODE *Node, const BOARD *Board) {

   int Moves;
   NODE *Son;

   if (EmptyNb(Board) < MIN_EMPTIES || IsFinished(Board)) return FALSE;

   Moves = Mobility(Board,Board->Colour);
   if (Moves == 0) Moves = 1;

   for (Son = Node->Son; Son != NULL; Son = Son->Brother) {
      if (IsLeaf(Son)) return FALSE;
      Moves--;
   }

   return Moves != 0;
}

/* StoreBest() */

static void StoreBest(NODE *Node, const BEST_MOVE *Best) {

   Node->Depth = Best->Depth;
   Node->Eval  = Best->BookValue;
}

/* BoardToPath() */

static int BoardToPath(const BOARD *Board, PATH *Path) {

   int   Square;
   BOARD Start[1], Goal[1];

   if (Root == NULL) return FALSE;

   ClearBoard(Start);
   ClearBoard(Goal);
   Goal->Colour = Board->Colour;

   for (Path->Sym = 0; Path->Sym < 4; Path->Sym++) {

      for (Square = 0; Square < SQUARE_NB; Square++) {
         Goal->Square[Square] = Board->Square[Symmetry[Path->Sym][Square]];
      }

      Path->Node = BoardToPath2(Root,Start,Path->Move,Goal);

      if (Path->Node != NULL) return TRUE;
   }

   return FALSE;
}

/* BoardToPath2() */

static NODE *BoardToPath2(NODE *Node, BOARD *Board, int Move[], const BOARD *Goal) {

   NODE *Found;

   if (SameBoard(Board,Goal)) {
      *Move = NO_MOVE;
      return Node;
   }

   for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
      if ((Node->Move == PASS || Goal->Square[Node->Move] != EMPTY) && ! Node->IsLink) {
         DoMove(Board,Node->Move);
         Found = BoardToPath2(Node,Board,Move+1,Goal);
         UndoMove(Board,Node->Move);
         if (Found != NULL) {
            *Move = Node->Move;
            return Found;
         }
      }
   }

   return NULL;
}

/* NodeToPath() */

static int NodeToPath(const NODE *Node, PATH *Path) {

   if (Root == NULL) return FALSE;

   Path->Sym  = ID_SYM;
   Path->Node = NodeToPath2(Root,Path->Move,Node);

   return Path->Node != NULL;
}

/* NodeToPath2() */

static NODE *NodeToPath2(NODE *Node, int Move[], const NODE *Goal) {

   NODE *Found;

   if (Node == Goal) {
      *Move = NO_MOVE;
      return Node;
   }

   for (Node = Node->Son; Node != NULL; Node = Node->Brother) {
      if (! Node->IsLink) {
         Found = NodeToPath2(Node,Move+1,Goal);
         if (Found != NULL) {
            *Move = Node->Move;
            return Found;
         }
      }
   }

   return NULL;
}

/* AsciiToPath() */

static void AsciiToPath(const char *Line, PATH *Path) {

   int *PathMove;

   Path->Sym = ID_SYM;

   for (PathMove = Path->Move; toupper(*Line) >= 'A' && toupper(*Line) <= 'H'; PathMove++) {
      *PathMove = StringSquare(Line);
      Line += 2;
      while (isspace(*Line)) Line++;
   }

   *PathMove = NO_MOVE;
}

/* GameToPath() */

static int GameToPath(const GAME *Game, PATH *Path) {

   int *PathMove;
   const GAME_MOVE *GameMove;

   if (! BoardToPath(Game->StartBoard,Path)) {
      Warning("Game start board not in book");
      return FALSE;
   }

   for (PathMove = Path->Move; *PathMove != NO_MOVE; PathMove++)
      ;

   for (GameMove = &Game->Move[0]; GameMove->Move != NO_MOVE; GameMove++, PathMove++) {
      *PathMove = Symmetry[Path->Sym][GameMove->Move];
   }

   *PathMove = NO_MOVE;

   return TRUE;
}

/* PathToAscii() */

static void PathToAscii(const PATH *Path) {

   const int *Move;

   for (Move = Path->Move; *Move != NO_MOVE; Move++) {
      printf("%s ",SquareString[*Move]);
   }
}

/* End of Book.C */


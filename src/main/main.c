/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Defn.h"
#include "Graphics.h"

	/* Heap and Pointer Protection Stack Sizes. */
	/* These have moved to Defn.h and Platform.h */


	/* Global Variables:  For convenience all interpeter */
	/* global symbols are declared here.  This does not */
	/* include user interface symbols which are included */
	/* in separate platform dependent modules. */


			/* Memory Management */

int	R_NSize = R_NSIZE;		/* Size of cons cell heap */
int	R_VSize = R_VSIZE;		/* Size of the vector heap */
SEXP	R_NHeap;			/* Start of the cons cell heap */
SEXP	R_FreeSEXP;			/* Cons cell free list */
VECREC*	R_VHeap;			/* Base of the vector heap */
VECREC*	R_VTop;				/* Current top of the vector heap */
VECREC*	R_VMax;				/* bottom of R_alloc'ed heap */
long	R_Collected;			/* Number of free cons cells (after gc) */


			/* The Pointer Protection Stack */

int	R_PPStackSize = R_PPSSIZE;	/* The stack size (elements) */
int	R_PPStackTop;			/* The top of the stack */
SEXP*	R_PPStack;			/* The pointer protection stack */


			/* Evaluation Environment */

SEXP	R_GlobalEnv;			/* The "global" environment */
SEXP	R_CurrentExpr;			/* Currently evaluating expression */
SEXP	R_ReturnedValue;		/* Slot for return-ing values */
SEXP*	R_SymbolTable;			/* The symbol table */
RCNTXT	R_Toplevel;			/* Storage for the toplevel environment */
RCNTXT*	R_ToplevelContext;		/* The toplevel environment */
RCNTXT*	R_GlobalContext;		/* The global environment */
int	R_Visible;			/* Value visibility flag */
int	R_EvalDepth = 0;		/* Evaluation recursion depth */
int	R_EvalCount = 0;		/* Evaluation count */


			/* File Input/Output */

int	R_Interactive = 1;		/* Interactive? */
int	R_Quiet = 0;			/* Be Quiet */
int	R_Console;			/* Console active flag */
FILE*	R_Inputfile = NULL;		/* Current input flag */
FILE*	R_Consolefile = NULL;		/* Console output file */
FILE*	R_Outputfile = NULL;		/* Output file */
FILE*	R_Sinkfile = NULL;		/* Sink file */


			/* Objects Used In Parsing  */

SEXP	R_CommentSxp;			/* Comments accumulate here */
SEXP	R_ParseText;			/* Text to be parsed */
int	R_ParseCnt;			/* Count of lines of text to be parsed */
int	R_ParseError = 0;		/* Line where parse error occured */


			/* Special Values */

SEXP	R_NilValue;			/* The nil object */
SEXP	R_UnboundValue;			/* Unbound marker */
SEXP	R_MissingArg;			/* Missing argument marker */


			/* Symbol Table Shortcuts */

SEXP	R_Bracket2Symbol;		/* "[[" */
SEXP	R_BracketSymbol;		/* "[" */
SEXP	R_ClassSymbol;			/* "class" */
SEXP	R_DimNamesSymbol;		/* "dimnames" */
SEXP	R_DimSymbol;			/* "dim" */
SEXP	R_DollarSymbol;			/* "$" */
SEXP	R_DotsSymbol;			/* "..." */
SEXP	R_DropSymbol;			/* "drop" */
SEXP	R_LevelsSymbol;			/* "levels" */
SEXP	R_ModeSymbol;			/* "mode" */
SEXP	R_NamesSymbol;			/* "names" */
SEXP	R_NaRmSymbol;			/* "na.rm" */
SEXP	R_RowNamesSymbol;		/* "row.names" */
SEXP	R_SeedsSymbol;			/* ".Random.seed" */
SEXP	R_LastvalueSymbol;		/* ".Last.value" */
SEXP	R_TspSymbol;			/* "tsp" */


			/* Arithmetic Values */

double	R_tmp;				/* Temporary Value */
double	R_NaN;				/* NaN or -DBL_MAX */  
double	R_PosInf;			/* IEEE Inf or DBL_MAX */  
double	R_NegInf;			/* IEEE -Inf or -DBL_MAX */  
int	R_NaInt;			/* NA_INTEGER */
double	R_NaReal;			/* NA_REAL */
SEXP	R_NaString;			/* NA_STRING */


			/* Image Dump/Restore */

char	R_ImageName[256];	/* Default image name */
int	R_Unnamed = 1;		/* Use default name? */
int	R_DirtyImage = 0;	/* Current image dirty */
int	R_Init = 0;		/* Do we have an image loaded */


static int ParseBrowser(SEXP, SEXP);


void InitGlobalEnv()
{
	R_GlobalEnv = emptyEnv();
}



	/* This is the R read-eval-print loop.  If R_Console */
	/* is zero, it is assumed that R_Inputfile contains */
	/* a file descriptor which is to be read to end of file */
	/* If R_Console is one, input is taken from the console. */

static void R_Repl(SEXP rho, int savestack, int browselevel)
{
	int pflag = 1;
	R_CommentSxp = CONS(R_NilValue, R_NilValue);
	CAR(R_CommentSxp) = R_CommentSxp;

	for (;;) {
		R_PPStackTop = savestack;
		if (R_Console == 1 && pflag <= 3) {
			if(browselevel)
				yyprompt("Browse[%d]> ", browselevel);
			else
				yyprompt((char*) CHAR(STRING(GetOption(install("prompt"), R_NilValue))[0]));
		}
		R_CurrentExpr = NULL;
		yyinit();
		pflag = yyparse();
		if (pflag == 0) {
			if (R_Console == 0) {
				fclose(R_Inputfile);
				ResetConsole();
			}
			return;
		}
		if(browselevel) {
			if( !R_CurrentExpr )
				return;
			if( ParseBrowser(R_CurrentExpr, rho) ) {
				if (R_Console == 0) {
					fclose(R_Inputfile);
					ResetConsole();
				}
				return;
			}
		}
		if(pflag != 1 && pflag != 2) {
			if (R_CurrentExpr) {
				R_Visible = 0;
				R_EvalDepth = 0;
				PROTECT(R_CurrentExpr);
				RBusy(1);
				R_CurrentExpr = eval(R_CurrentExpr, rho);
				SYMVALUE(R_LastvalueSymbol) = R_CurrentExpr;
				UNPROTECT(1);
				if (R_Visible)
					PrintValueEnv(R_CurrentExpr, rho);
			}
		}
	}
}

	/* Main Loop: It is assumed that at this point that */
	/* operating system specific tasks (dialog window */
	/* creation etc) have been performed.  We can now */
	/* print a greeting, run the .First function and then */
	/* enter the read-eval-print loop. */


	/* The following variable must be external to mainloop *.
	/* because gcc -O seems to eliminate a local one? */

#ifndef Macintosh
FILE* R_OpenSysInitFile(void);
FILE* R_OpenInitFile(void);
#endif

static int doneit;

void mainloop()
{
	SEXP cmd;

		/* Print a platform and version dependent */
		/* greeting and a pointer to the copyleft. */

	if(!R_Quiet)
		PrintGreeting();


		/* Initialize the interpreter's */
		/* internal structures. */

	InitMemory();
	InitNames();
	InitGlobalEnv();
	InitFunctionHashing();
	InitOptions();
	InitEd();
	InitArithmetic();
	InitColors();


		/* Initialize the global context for error handling. */
		/* This provides a target for any non-local gotos */
		/* which occur during error handling */

	R_Toplevel.nextcontext = NULL;
	R_Toplevel.callflag = CTXT_TOPLEVEL;
	R_Toplevel.cstacktop = 0;
	R_Toplevel.promargs = R_NilValue;
	R_Toplevel.call = R_NilValue;
	R_Toplevel.cloenv = R_NilValue;
	R_Toplevel.sysparent = R_NilValue;
	R_Toplevel.conexit = R_NilValue;
	R_Toplevel.cend = NULL;
	R_GlobalContext = R_ToplevelContext = &R_Toplevel;


		/* On initial entry we open the base language */
		/* library and begin by running the repl on it. */
		/* If there is an error we pass on to the repl. */
		/* Perhaps it makes more sense to quit gracefully? */

	R_Console = 0;
	R_Inputfile = R_OpenLibraryFile("base");
	if(R_Inputfile == NULL) {
		suicide("unable to open the base library\n");
	}
	doneit = 0;
	setjmp(R_Toplevel.cjmpbuf);
	R_GlobalContext = R_ToplevelContext = &R_Toplevel;
	signal(SIGINT, onintr);
	if(!doneit) {
		doneit = 1;
#ifdef OLD
		R_Repl(R_GlobalEnv, 0, 0);
#else
		R_Repl(R_NilValue, 0, 0);
#endif
	}

		/* This is where we try to load a user's */
		/* saved data.  The right thing to do here */
		/* is very platform dependent.  E.g. Under */
		/* Unix we look in a special hidden file and */
		/* on the Mac we look in any documents which */
		/* might have been double clicked on or dropped */
		/* on the application */

	R_Console = 1;
	doneit = 0;
	setjmp(R_Toplevel.cjmpbuf);
	R_GlobalContext = R_ToplevelContext = &R_Toplevel;
	signal(SIGINT, onintr);
	if(!doneit) {
		doneit = 1;
		R_InitialData();
	}

#ifndef Macintosh 
		/* This is where we source the system-wide */
		/* profile file.  If there is an error */
		/* we drop through to further processing. */

	R_Console = 0;
	R_Inputfile = R_OpenSysInitFile();
	if(R_Inputfile != NULL) {
		doneit = 0;
		setjmp(R_Toplevel.cjmpbuf);
		R_GlobalContext = R_ToplevelContext = &R_Toplevel;
		signal(SIGINT, onintr);
		if(!doneit) {
			doneit = 1;
			R_Repl(R_NilValue, 0, 0);
		}
	}	

		/* This is where we source the user's */
		/* profile file.  If there is an error */
		/* we drop through to further processing. */

	R_Console = 0;
	R_Inputfile = R_OpenInitFile();
	if(R_Inputfile != NULL) {
		doneit = 0;
		setjmp(R_Toplevel.cjmpbuf);
		R_GlobalContext = R_ToplevelContext = &R_Toplevel;
		signal(SIGINT, onintr);
		if(!doneit) {
			doneit = 1;
			R_Repl(R_GlobalEnv, 0, 0);
		}
	}	
#endif

		/* Initial Loading is done.  At this point */
		/* we try to invoke the .First Function. */
		/* If there is an error we continue */

	R_Console = 1;
	doneit = 0;
	setjmp(R_Toplevel.cjmpbuf);
	R_GlobalContext = R_ToplevelContext = &R_Toplevel;
	signal(SIGINT, onintr);
	if(!doneit) {
		doneit = 1;
		PROTECT(cmd = install(".First"));
		R_CurrentExpr = findVar(cmd, R_GlobalEnv);
		if (R_CurrentExpr != R_UnboundValue && TYPEOF(R_CurrentExpr) == CLOSXP) {
			PROTECT(R_CurrentExpr = lang1(cmd));
			R_CurrentExpr = eval(R_CurrentExpr, R_GlobalEnv);
			UNPROTECT(1);
		}
		UNPROTECT(1);
	}

		/* Here is the real R read-eval-loop. */
		/* We handle the console until  end-of-file. */

	R_Console = 1;
	setjmp(R_Toplevel.cjmpbuf);
	R_GlobalContext = R_ToplevelContext = &R_Toplevel;
	signal(SIGINT, onintr);
	R_Repl(R_GlobalEnv, 0, 0);
	Rprintf("\n");


		/* We have now exited from the read-eval loop. */
		/* Now we run the .Last function and exit. */
		/* Errors here should kick us back into the repl. */

	R_GlobalContext = R_ToplevelContext = &R_Toplevel;
	PROTECT(cmd = install(".Last"));
	R_CurrentExpr = findVar(cmd, R_GlobalEnv);
	if (R_CurrentExpr != R_UnboundValue && TYPEOF(R_CurrentExpr) == CLOSXP) {
		PROTECT(R_CurrentExpr = lang1(cmd));
		R_CurrentExpr = eval(R_CurrentExpr, R_GlobalEnv);
		UNPROTECT(1);
	}
	UNPROTECT(1);
	RCleanUp(1); /* query save */
}

int R_BrowseLevel = 0;

static int ParseBrowser(SEXP CExpr, SEXP rho) 
{
	int rval=0;

	if( isSymbol(CExpr) ) {
		if(!strcmp(CHAR(PRINTNAME(CExpr)),"n")) {
			DEBUG(rho)=1;
			rval=1;
		}
		if(!strcmp(CHAR(PRINTNAME(CExpr)),"c")) {
			rval=1;
			DEBUG(rho)=0;
		}
		if(!strcmp(CHAR(PRINTNAME(CExpr)),"cont")) {
			rval=1;
			DEBUG(rho)=0;
		}
	}
	return rval;
}

SEXP do_browser(SEXP call, SEXP op, SEXP args, SEXP rho)
{
	RCNTXT *saveToplevelContext;
	RCNTXT *saveGlobalContext;
	RCNTXT thiscontext, returncontext;
	int savestack;
	int savebrowselevel;
	int saveEvalDepth;
	int pflag = 1;
	SEXP topExp, tmp;

		/* Save the evaluator state information */
		/* so that it can be restored on exit. */

	savebrowselevel = R_BrowseLevel + 1;
	savestack = R_PPStackTop;
	PROTECT(topExp = R_CurrentExpr);
	saveToplevelContext = R_ToplevelContext;
	saveGlobalContext = R_GlobalContext;
	saveEvalDepth = R_EvalDepth;

		/* Here we establish two contexts.  The first */
		/* of these provides a target for return */
		/* statements which a user might type at the */
		/* browser prompt.  The (optional) second one */
		/* acts as a target for error returns. */

	begincontext(&returncontext, CTXT_BROWSER, call, rho, R_NilValue, R_NilValue);
	if (setjmp(returncontext.cjmpbuf)) {
		tmp = R_NilValue;		/* R_ReturnedValue */
	}
	else {
		begincontext(&thiscontext, CTXT_TOPLEVEL, R_NilValue, rho, R_NilValue, R_NilValue);
		setjmp(thiscontext.cjmpbuf);
		R_GlobalContext = R_ToplevelContext = &thiscontext;
		R_BrowseLevel = savebrowselevel;
		R_Repl(rho, savestack, R_BrowseLevel);
		tmp = R_NilValue;
		endcontext(&thiscontext);
	}
	endcontext(&returncontext);

		/* Reset the interpreter state. */

	R_CurrentExpr = topExp;
	UNPROTECT(1);
	R_PPStackTop = savestack;
	R_CurrentExpr = topExp;
	R_ToplevelContext = saveToplevelContext;
	R_GlobalContext = saveGlobalContext;
	R_EvalDepth = saveEvalDepth;
	R_BrowseLevel--;
	return tmp;
}
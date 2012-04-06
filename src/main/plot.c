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
#include "Mathlib.h"
#include "Graphics.h"

	/* Coordinate Mappings */
	/* Linear/Logarithmic Scales */

static double (*xt) (double);
static double (*yt) (double);

static double Log10(double x)
{
	return (FINITE(x) && x > 0.0) ? log10(x) : NA_REAL;
}

static double Ident(double x)
{
	return x;
}

void NewFrameConfirm()
{
	int c;
	REprintf("Next plot? ");
	yyprompt("");
	while((c = cget()) != '\n' && c != EOF)
		;
}


	/* Device Startup */

SEXP do_device(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP s;
	char *device;
	int i, ncpars, nnpars;
	char *cpars[20];
	double *npars;

		/* NO GARBAGE COLLECTS ALLOWED HERE*/
		/* WE ARE USING REAL POINTERS */
		/* SWITCH TO R_ALLOCING IF NECESSARY */

	checkArity(op, args);

	s = CAR(args);
	if (!isString(s) || length(s) <= 0)
		errorcall(call, "device name must be a character string\n");
	device = CHAR(STRING(s)[0]);

	s = CADR(args);
	if (!isString(s) || length(s) > 20)
		errorcall(call, "invalid device driver parameters\n");
	ncpars = LENGTH(s);
	for(i=0 ; i<LENGTH(s) ; i++)
		cpars[i] = CHAR(STRING(s)[i]);

	s = CADDR(args);
	if (!isReal(CADDR(args)))
		errorcall(call, "width and height must be numeric\n");
	nnpars = LENGTH(s);

	if( !strcmp(device,"X11") )
		for(i=0 ; i<nnpars ; i++ ) 
			if(!FINITE(REAL(s)[i]) || REAL(s)[i] <= 0)
				errorcall(call, "invalid device driver parameter\n");
	npars = REAL(s);

	if (!SetDevice(device, cpars, ncpars, npars, nnpars))
		errorcall(call, "unable to start device %s\n", device);

	xt = Ident;
	yt = Ident;

		/* GARBAGE COLLECTS ARE OK FROM HERE */

	s = mkString(CHAR(STRING(CAR(args))[0]));
	gsetVar(install(".Device"), s, R_NilValue);

	return CAR(args);
}

SEXP do_devoff(SEXP call, SEXP op, SEXP args, SEXP env)
{
	checkArity(op, args);
	KillDevice();
	gsetVar(install(".Device"), R_NilValue, R_NilValue);
	return R_NilValue;
}


	/* Saving and restoring of "inline" graphical */
	/* parameters.  These are the ones which can */
	/* be specified as a arguments to high-level */
	/* graphics functions */

static double	adjsave;	/* adj */
static int	annsave;	/* ann */
static int	btysave;	/* bty */
static double	cexsave;	/* cex */
static double	cexmainsave;	/* cex.main */
static double	cexlabsave;	/* cex.lab */
static double	cexsubsave;	/* cex.sub */
static double	cexaxissave;	/* cex.axis */
static int	colsave;	/* col */
static int	fgsave;		/* fg */
static int	bgsave;		/* bg */
static int	colmainsave;	/* col.main */
static int	collabsave;	/* col.lab */
static int	colsubsave;	/* col.sub */
static int	colaxissave;	/* col.axis */
static double	crtsave;	/* character rotation */
static int	fontsave;	/* font */
static int	fontmainsave;	/* font.main */
static int	fontlabsave;	/* font.lab */
static int	fontsubsave;	/* font.sub */
static int	fontaxissave;	/* font.axis */
/* static int	csisave;	/* line spacing in inches */
static int	errsave;	/* error mode */
static int	labsave[3];	/* axis labelling parameters */
static int	lassave;	/* label style */
static int	ltysave;	/* line type */
static double	lwdsave;	/* line width */
static int	mgpsave[3];	/* margin position for annotation */
static double	mkhsave;	/* mark height */
static int	pchsave;	/* plotting character */
static double	srtsave;	/* string rotation */
static double	tcksave;	/* tick mark length */
static double	xaxpsave[3];	/* x axis parameters */
static int	xaxssave;	/* x axis calculation style */
static int	xaxtsave;	/* x axis type */
static int	xpdsave;	/* clipping control */
static double	yaxpsave[3];	/* y axis parameters */
static int	yaxssave;	/* y axis calculation style */
static int	yaxtsave;	/* y axis type */

static void SavePars()
{
	adjsave = GP->adj;
	annsave = GP->ann;
	btysave = GP->bty;
	cexsave = GP->cex;
	cexlabsave = GP->cexlab;
	cexmainsave = GP->cexmain;
	cexsubsave = GP->cexsub;
	cexaxissave = GP->cexaxis;
	colsave = GP->col;
	fgsave = GP->fg;
	bgsave = GP->bg;
	collabsave = GP->collab;
	colmainsave = GP->colmain;
	colsubsave = GP->colsub;
	colaxissave = GP->colaxis;
	crtsave = GP->crt;
	errsave = GP->err;
	fontsave = GP->font;
	fontmainsave = GP->fontmain;
	fontlabsave = GP->fontlab;
	fontsubsave = GP->fontsub;
	fontaxissave = GP->fontaxis;
	/* csisave = GP->csi; */
	labsave[0] = GP->lab[0];
	labsave[1] = GP->lab[1];
	labsave[2] = GP->lab[2];
	lassave = GP->las;
	ltysave = GP->lty;
	lwdsave = GP->lwd;
	mgpsave[0] = GP->mgp[0];
	mgpsave[1] = GP->mgp[1];
	mgpsave[2] = GP->mgp[2];
	mkhsave = GP->mkh;
	pchsave = GP->pch;
	srtsave = GP->srt;
	tcksave = GP->tck;
	xaxpsave[0] = GP->xaxp[0];
	xaxpsave[1] = GP->xaxp[1];
	xaxpsave[2] = GP->xaxp[2];
	xaxssave = GP->xaxs;
	xaxtsave = GP->xaxt;
	xpdsave = GP->xpd;
	yaxpsave[0] = GP->yaxp[0];
	yaxpsave[1] = GP->yaxp[1];
	yaxpsave[2] = GP->yaxp[2];
	yaxssave = GP->yaxs;
	yaxtsave = GP->yaxt;
}

static void RestorePars()
{
	GP->adj = adjsave;
	GP->ann = annsave;
	GP->bty = btysave;
	GP->cex = cexsave;
	GP->cexlab = cexlabsave;
	GP->cexmain = cexmainsave;
	GP->cexsub = cexsubsave;
	GP->cexaxis = cexaxissave;
	GP->col = colsave;
	GP->fg = fgsave;
	GP->bg = bgsave;
	GP->collab = collabsave;
	GP->colmain = colmainsave;
	GP->colsub = colsubsave;
	GP->colaxis = colaxissave;
	GP->crt = crtsave;
	GP->err = errsave;
	GP->font = fontsave;
	GP->fontmain = fontmainsave;
	GP->fontlab = fontlabsave;
	GP->fontsub = fontsubsave;
	GP->fontaxis = fontaxissave;
	/* GP->csi = csisave; */
	GP->lab[0] = labsave[0];
	GP->lab[1] = labsave[1];
	GP->lab[2] = labsave[2];
	GP->las = lassave;
	GP->lty = ltysave;
	GP->lwd = lwdsave;
	GP->mgp[0] = mgpsave[0];
	GP->mgp[1] = mgpsave[1];
	GP->mgp[2] = mgpsave[2];
	GP->mkh = mkhsave;
	GP->pch = pchsave;
	GP->srt = srtsave;
	GP->tck = tcksave;
	GP->xaxp[0] = xaxpsave[0];
	GP->xaxp[1] = xaxpsave[1];
	GP->xaxp[2] = xaxpsave[2];
	GP->xaxs = xaxssave;
	GP->xaxt = xaxtsave;
	GP->xpd = xpdsave;
	GP->yaxp[0] = yaxpsave[0];
	GP->yaxp[1] = yaxpsave[1];
	GP->yaxp[2] = yaxpsave[2];
	GP->yaxs = yaxssave;
	GP->yaxt = yaxtsave;
}

void Specify2(char*, SEXP);

void ProcessInlinePars(SEXP s)
{
	if(isList(s)) {
		while(s != R_NilValue) {
			if(isList(CAR(s)))
				ProcessInlinePars(CAR(s));
			else if(TAG(s) != R_NilValue)
				Specify2(CHAR(PRINTNAME(TAG(s))), CAR(s));
			s = CDR(s);
		}
	}
}

	/* GetPar is intended for looking through a list */
	/* typically that bound to ... for a particular */
	/* parameter value.  This is easier than trying */
	/* to match every graphics parameter in argument */
	/* lists and passing them explicitly. */

SEXP GetPar(char *which, SEXP parlist)
{
	SEXP w, p;
	w = install(which);
	for(p=parlist ; p!=R_NilValue ; p=CDR(p)) {
		if(TAG(p) == w)
			return CAR(p);
	}
	return R_NilValue;
}

SEXP FixupPch(SEXP pch)
{
	int i, n;
	SEXP ans;

	if(length(pch) == 0) {
		ans = allocVector(INTSXP, n=1);
		INTEGER(ans)[0] = GP->pch;
	}
	else if(isList(pch)) {
		ans = allocVector(INTSXP, n=length(pch));
		for(i=0 ; pch != R_NilValue ;  pch = CDR(pch))
			INTEGER(ans)[i++] = asInteger(CAR(pch));
	}
	else if(isInteger(pch)) {
		ans = allocVector(INTSXP, n=length(pch));
		for(i=0 ; i<n ; i++)
			INTEGER(ans)[i] = INTEGER(pch)[i];
	}
	else if(isReal(pch)) {
		ans = allocVector(INTSXP, n=length(pch));
		for(i=0 ; i<n ; i++)
			INTEGER(ans)[i] = FINITE(REAL(pch)[i]) ?
						REAL(pch)[i] : NA_INTEGER;
	}
	else if(isString(pch)) {
		ans = allocVector(INTSXP, n=length(pch));
		for(i=0 ; i<n ; i++)
			INTEGER(ans)[i] = CHAR(STRING(pch)[i])[0];
	}
	else error("invalid plotting symbol\n");
	for(i=0 ; i<n ; i++) {
		if(INTEGER(ans)[i] < 0)
			INTEGER(ans)[i] = GP->pch;
	}
	return ans;
}

SEXP FixupLty(SEXP lty)
{
	int i, n;
	SEXP ans;
	if(length(lty) == 0) {
		ans = allocVector(INTSXP, 1);
		INTEGER(ans)[0] = GP->lty;
	}
	else {
		ans = allocVector(INTSXP, n=length(lty));
		for(i=0 ; i<n; i++)
			INTEGER(ans)[i] = LTYpar(lty, i);
	}
	return ans;
}

SEXP FixupFont(SEXP font)
{
	int i, k, n;
	SEXP ans;
	if(length(font) == 0) {
		ans = allocVector(INTSXP, 1);
		INTEGER(ans)[0] = NA_INTEGER;
	}
	else if(isInteger(font)) {
		ans = allocVector(INTSXP, n=length(font));
		for(i=0 ; i<n; i++) {
			k = INTEGER(font)[i];
			if(k < 1 || k > 4) k = NA_INTEGER;
			INTEGER(ans)[i] = k;
		}
	}
	else if(isReal(font)) {
		ans = allocVector(INTSXP, n=length(font));
		for(i=0 ; i<n; i++) {
			k = REAL(font)[i];
			if(k < 1 || k > 4) k = NA_INTEGER;
			INTEGER(ans)[i] = k;
		}
	}
	else error("invalid font specification\n");
	return ans;
}

SEXP FixupCol(SEXP col)
{
	int i, n;
	SEXP ans;

	if(length(col) == 0) {
		ans = allocVector(INTSXP, 1);
		INTEGER(ans)[0] = NA_INTEGER;
	}
	else if(isList(col)) {
		ans = allocVector(INTSXP, n=length(col));
		for(i=0 ; i<n; i++) {
			INTEGER(ans)[i] = RGBpar(CAR(col), 0);
			col = CDR(col);
		}
	}
	else {
		ans = allocVector(INTSXP, n=length(col));
		for(i=0 ; i<n; i++)
			INTEGER(ans)[i] = RGBpar(col, i);
	}
	return ans;
}

SEXP FixupCex(SEXP cex)
{
	SEXP ans;
	int i, n;
	double c;

	if(length(cex) == 0) {
		ans = allocVector(REALSXP, 1);
		REAL(ans)[0] = NA_REAL;
	}
	else if(isReal(cex)) {
		ans = allocVector(REALSXP, n=length(cex));
		for(i=0 ; i<n; i++) {
			c = REAL(cex)[i];
			if(FINITE(c) && c > 0)
				REAL(ans)[i] = c;
			else
				REAL(ans)[i] = NA_REAL;
		}
	}
	else if(isInteger(cex)) {
		ans = allocVector(REALSXP, n=length(cex));
		for(i=0 ; i<n; i++) {
			c = INTEGER(cex)[i];
			if(c == NA_INTEGER || c <= 0)
				c = NA_REAL;
			REAL(ans)[i] = c;
		}
	}
	return ans;
}

	/*  plot.new(ask)  */
	/*  create a new plot  */

SEXP do_plot_new(SEXP call, SEXP op, SEXP args, SEXP env)
{
	int ask, asksave;
	checkArity(op, args);
	ask = asLogical(CAR(args));	
	if(ask == NA_LOGICAL) ask = DP->ask;
	asksave = GP->ask;
	GP->ask = ask;
	GNewPlot();
	DP->xlog = GP->xlog = 0;
	DP->ylog = GP->ylog = 0;
	xt = Ident;
	yt = Ident;
	GScale(0.0, 1.0, 1);
	GScale(0.0, 1.0, 2);
	GMapWin2Fig();
	GSetState(1);
	GP->ask = asksave;
	return R_NilValue;
}


	/*  plot.window(xlim, ylim, log) */
	/*  define world coordinates  */

SEXP do_plot_window(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP xlim, ylim, log;
	double xmin, xmax, ymin, ymax;
	char *p;

	/* checkArity(op, args); */
	if(length(args) < 3)
		errorcall(call, "at least 3 arguments required\n");

	xlim = CAR(args);
	if(!isNumeric(xlim) || LENGTH(xlim) != 2)
		errorcall(call, "invalid xlim\n");
	args = CDR(args);

	ylim = CAR(args);
	if(!isNumeric(ylim) || LENGTH(ylim) != 2)
		errorcall(call, "invalid ylim\n");
	args = CDR(args);

	log = CAR(args);
	if (!isString(log))
		error("invalid \"log=\" specification\n");
	p = CHAR(STRING(log)[0]);
	while (*p) {
		switch (*p) {
		case 'x':
			DP->xlog = GP->xlog = 1;
			xt = Log10;
			break;
		case 'y':
			DP->ylog = GP->ylog = 1;
			yt = Log10;
			break;
		default:
			error("invalid \"log=\" specification\n");
		}
		p++;
	}
	args = CDR(args);

	SavePars();
	ProcessInlinePars(args);

	if(isInteger(xlim)) {
		if(INTEGER(xlim)[0] == NA_INTEGER || INTEGER(xlim)[1] == NA_INTEGER)
			errorcall(call, "NAs not allowed in xlim\n");
		xmin = INTEGER(xlim)[0];
		xmax = INTEGER(xlim)[1];
	}
	else {
		if(!FINITE(REAL(xlim)[0]) || !FINITE(REAL(xlim)[1]))
			errorcall(call, "NAs not allowed in xlim\n");
		xmin = REAL(xlim)[0];
		xmax = REAL(xlim)[1];
	}
	if(isInteger(ylim)) {
		if(INTEGER(ylim)[0] == NA_INTEGER || INTEGER(ylim)[1] == NA_INTEGER)
			errorcall(call, "NAs not allowed in ylim\n");
		ymin = INTEGER(ylim)[0];
		ymax = INTEGER(ylim)[1];
	}
	else {
		if(!FINITE(REAL(ylim)[0]) || !FINITE(REAL(ylim)[1]))
			errorcall(call, "NAs not allowed in ylim\n");
		ymin = REAL(ylim)[0];
		ymax = REAL(ylim)[1];
	}
	GCheckState();
	GScale(xmin, xmax, 1);
	GScale(ymin, ymax, 2);
	GMapWin2Fig();
	RestorePars();
	return R_NilValue;
}

static void GetAxisLimits(double left, double right, double *low, double *high)
{
	double eps;
	if(left <= right) {
		eps = FLT_EPSILON * (right - left);
		if(eps == 0) eps = 0.5 * FLT_EPSILON;
		*low = left - eps;
		*high = right + eps;
	}
	else {
		eps = FLT_EPSILON * (left - right);
		if(eps == 0) eps = 0.5 * FLT_EPSILON;
		*low = right - eps;
		*high = left + eps;
	}
}

	/* axis(which, at, labels, ...) -- draw an axis */

SEXP do_axis(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP at, lab;
	int col, fg, i, n, which;
	double x, y, xc, yc, xtk, ytk, tnew, tlast, cwid;
	double gap, labw, low, high;

		/*  Initial checks  */

	GCheckState(); 
	if(length(args) < 3)
		errorcall(call, "too few arguments");

		/*  Required arguments  */

	which = asInteger(CAR(args));
	if (which < 1 || which > 4)
		errorcall(call, "invalid axis number\n");
	args = CDR(args);
	
	internalTypeCheck(call, at = CAR(args), REALSXP);
	args = CDR(args);

	internalTypeCheck(call, lab = CAR(args), STRSXP);
	if (LENGTH(at) != LENGTH(lab))
		errorcall(call, "location and label lengths differ\n");
	n = LENGTH(at);
	args = CDR(args);

		/*  Process any optional graphical parameters  */

	R_Visible = 0;
	SavePars();
	GP->adj = 0.5;
	GP->xpd = 1;
	ProcessInlinePars(args);
	GP->font = GP->fontaxis;
	GP->cex = GP->cex * GP->cexbase;
	col = GP->col;
	fg = GP->fg;

		/*  Check the axis type parameter  */
		/*  If it is 'n', there is nothing to do  */

	if(which == 1 || which == 3) {
		if(GP->xaxt == 'n') {
			RestorePars();
			return R_NilValue;
		}
	}
	else if(which == 2 || which == 4) {
		if(GP->yaxt == 'n') {
			RestorePars();
			return R_NilValue;
		}
	}
	else errorcall(call, "invalid \"which\" value\n");

		/* Compute the ticksize in NDC units */

	xc = fabs(GP->mex * GP->cexbase * GP->cra[1] * GP->asp / GP->fig2dev.bx);
	yc = fabs(GP->mex * GP->cexbase * GP->cra[1] / GP->fig2dev.by);
	xtk = 0.5 * xc;
	ytk = 0.5 * yc;
	x = GP->plt[0];
	y = GP->plt[2];

		/*  Draw the axis  */

	GMode(1);
	switch (which) {
	case 1:
	case 3:
		GetAxisLimits(GP->plt[0], GP->plt[1], &low, &high);
		if (which == 3) {
			y = GP->plt[3];
			ytk = -ytk;
		}
		GP->col = fg;
		GStartPath();
		GMoveTo(XMAP(xt(REAL(at)[0])), y);
		GLineTo(XMAP(xt(REAL(at)[n - 1])), y);
		for (i = 0; i < n; i++) {
			x = XMAP(xt(REAL(at)[i]));
			if (low <= x && x <= high) {
				GMoveTo(x, y);
				GLineTo(x, y - ytk);
			}
		}
		GEndPath();
		GP->col = GP->colaxis;
		tlast = -1.0;
		gap = GStrWidth("m", 2);	/* FIXUP x/y distance */
		for (i = 0; i < n; i++) {
			x = XMAP(xt(REAL(at)[i]));
			labw = GStrWidth(CHAR(STRING(lab)[i]), 2);
			tnew = x - 0.5 * labw;
			if (tnew - tlast >= gap) {
				GMtext(CHAR(STRING(lab)[i]), which, GP->mgp[1], 0, xt(REAL(at)[i]), GP->las);
				tlast = x + 0.5 *labw;
			}
		}
		break;
	case 2:
	case 4:
		GetAxisLimits(GP->plt[2], GP->plt[3], &low, &high);
		if (which == 4) {
			x = GP->plt[1];
			xtk = -xtk;
		}
		GP->col = fg;
		GStartPath();
		GMoveTo(x, YMAP(yt(REAL(at)[0])));
		GLineTo(x, YMAP(yt(REAL(at)[n - 1])));
		for (i = 0; i < n; i++) {
			y = YMAP(yt(REAL(at)[i]));
			if (low <= y && y <= high) {
				GMoveTo(x, y);
				GLineTo(x - xtk, y);
			}
		}
		GEndPath();
		GP->col = GP->colaxis;
		gap = GStrWidth("m", 2);
		gap = yInchtoFig(xFigtoInch(gap));
		tlast = -1.0;
		cwid = GP->cex * fabs(GP->cra[0] / GP->ndc2dev.by);
		for (i = 0; i < n; i++) {
			y = YMAP(yt(REAL(at)[i]));
			labw = GStrWidth(CHAR(STRING(lab)[i]), 2);
			labw = yInchtoFig(xFigtoInch(labw));
			tnew = y - 0.5 * labw;
			if (tnew - tlast >= gap) {
				GMtext(CHAR(STRING(lab)[i]), which, GP->mgp[1], 0, yt(REAL(at)[i]), GP->las);
				tlast = y + 0.5 *labw;
			}
		}
		break;
	}

	GMode(0);
	RestorePars();
	return R_NilValue;
}

	/*  plot.xy(xy, type, pch, lty, col, cex, ...)  */
	/*  plot points or lines of various types  */

SEXP do_plot_xy(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sxy, sx, sy, pch, cex, col, bg, lty;
	double *x, *y, xold, yold, xx, yy;
	int i, n, npch, ncex, ncol, nbg, nlty, type;

		/* Basic Checks */

	GCheckState(); 
	if(length(args) < 6)
		errorcall(call, "too few arguments\n");

		/* Required Arguments */

	sxy = CAR(args);
	if (!isList(sxy) || length(sxy) < 2)
		errorcall(call, "invalid plotting structure\n");
	internalTypeCheck(call, sx = CAR(sxy), REALSXP);
	internalTypeCheck(call, sy = CADR(sxy), REALSXP);
	if (LENGTH(sx) != LENGTH(sy))
		error("x and y lengths differ for plot\n");
	n = LENGTH(sx);
	args = CDR(args);

	if(isNull(CAR(args))) type = 'p';
	else {
		if(isString(CAR(args)) && LENGTH(CAR(args)) == 1)
			type = CHAR(STRING(CAR(args))[0])[0];
		else errorcall(call, "invalid plot type\n");
	}
	args = CDR(args);

	PROTECT(pch = FixupPch(CAR(args)));
	npch = length(pch);
	args = CDR(args);

	PROTECT(lty = FixupLty(CAR(args)));
	nlty = length(lty);
	args = CDR(args);

	PROTECT(col = FixupCol(CAR(args)));
	ncol = LENGTH(col);
	args = CDR(args);

	PROTECT(bg = FixupCol(CAR(args)));
	nbg = LENGTH(bg);
	args = CDR(args);

	PROTECT(cex = FixupCex(CAR(args)));
	ncex = LENGTH(cex);
	args = CDR(args);

		/* Miscellaneous Graphical Parameters */

	SavePars();
	ProcessInlinePars(args);

	x = REAL(sx);
	y = REAL(sy);

	if(nlty && INTEGER(lty)[0] != NA_INTEGER)
		GP->lty = INTEGER(lty)[0];

	if(ncex && FINITE(REAL(cex)[0]))
		GP->cex = GP->cexbase * REAL(cex)[0];
	else
		GP->cex = GP->cexbase;

	GMode(1);
	GClip();

		/* lines and overplotted lines and points */

	if (type == 'l' || type == 'o') {
		GP->col = INTEGER(col)[0];
		xold = NA_REAL;
		yold = NA_REAL;
		GStartPath();
		for (i = 0; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xold) && FINITE(yold) && FINITE(xx) && FINITE(yy)) {
				GLineTo(XMAP(xx), YMAP(yy));
			}
			else if (FINITE(xx) && FINITE(yy))
				GMoveTo(XMAP(xx), YMAP(yy));
			xold = xx;
			yold = yy;
		}
		GEndPath();
	}

		/* points connected with broken lines */

	if(type == 'b' || type == 'c') {
		double d, f, x0, x1, xc, y0, y1, yc;
		d = 0.5 * GP->cex * GP->cra[1] * GP->ipr[1];
		xc = xNDCtoInch(GP->fig2dev.bx / GP->ndc2dev.bx) * (GP->plt[1] - GP->plt[0]);
		yc = yNDCtoInch(GP->fig2dev.by / GP->ndc2dev.by) * (GP->plt[3] - GP->plt[2]);
		xc = xc / (GP->usr[1] - GP->usr[0]);
		yc = yc / (GP->usr[3] - GP->usr[2]);
		GP->col = INTEGER(col)[0];
		xold = NA_REAL;
		yold = NA_REAL;
		GStartPath();
		for (i = 0; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xold) && FINITE(yold) && FINITE(xx) && FINITE(yy)) {
				if((f = d/hypot(xc * (xx-xold), yc * (yy-yold))) < 0.5) {
					x0 = xold + f * (xx - xold);
					y0 = yold + f * (yy - yold);
					x1 = xx + f * (xold - xx);
					y1 = yy + f * (yold - yy);
					GMoveTo(XMAP(x0), YMAP(y0));
					GLineTo(XMAP(x1), YMAP(y1));
				}
			}
			xold = xx;
			yold = yy;
		}
		GEndPath();
	}

	if (type == 's') {
		GP->col = INTEGER(col)[0];
		xold = xt(x[0]);
		yold = xt(y[0]);
		GStartPath();
		if (FINITE(xold) && FINITE(yold))
			GMoveTo(XMAP(xold), YMAP(yold));
		for (i = 1; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xold) && FINITE(yold) && FINITE(xx) && FINITE(yy)) {
				GLineTo(XMAP(xx), YMAP(yold));
				GLineTo(XMAP(xx), YMAP(yy));
			}
			else if (FINITE(x[i]) && FINITE(y[i]))
				GMoveTo(XMAP(xx), YMAP(yy));
			xold = xx;
			yold = yy;
		}
		GEndPath();
	}

	if (type == 'S') {
		GP->col = INTEGER(col)[0];
		xold = xt(x[0]);
		yold = xt(y[0]);
		GStartPath();
		if (FINITE(xold) && FINITE(yold))
			GMoveTo(XMAP(xold), YMAP(yold));
		for (i = 1; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xold) && FINITE(yold) && FINITE(xx) && FINITE(yy)) {
				GLineTo(XMAP(xold), YMAP(yy));
				GLineTo(XMAP(xx), YMAP(yy));
			}
			else if (FINITE(x[i]) && FINITE(y[i]))
				GMoveTo(XMAP(xx), YMAP(yy));
			xold = xx;
			yold = yy;
		}
		GEndPath();
	}

	if (type == 'h') {
		GP->col = INTEGER(col)[0];
		for (i = 0; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xx) && FINITE(yy)) {
				GStartPath();
				GMoveTo(XMAP(xx), YMAP(yt(0.0)));
				GLineTo(XMAP(xx), YMAP(yy));
				GEndPath();
			}
		}
	}

	GP->lty = ltysave;
	if (type == 'p' || type == 'b' || type == 'o') {
		for (i = 0; i < n; i++) {
			xx = xt(x[i]);
			yy = yt(y[i]);
			if (FINITE(xx) && FINITE(yy)) {
				GP->col = INTEGER(col)[i % ncol];
				GP->bg = INTEGER(bg)[i % nbg];
				GSymbol(XMAP(xx), YMAP(yy), INTEGER(pch)[i % npch]);
			}
		}
	}
	GMode(0);
	RestorePars();
	UNPROTECT(5);
	return R_NilValue;
}

static void xypoints(SEXP call, SEXP args, int *n)
{
	int k;

	if (!isNumeric(CAR(args)) || (k = LENGTH(CAR(args))) <= 0)
		errorcall(call, "first argument invalid\n");
	CAR(args) = coerceVector(CAR(args), REALSXP);
	*n = k;
	args = CDR(args);

	if (!isNumeric(CAR(args)) || (k = LENGTH(CAR(args))) <= 0)
		errorcall(call, "second argument invalid\n");
	CAR(args) = coerceVector(CAR(args), REALSXP);
	if (k > *n) *n = k;
	args = CDR(args);

	if (!isNumeric(CAR(args)) || (k = LENGTH(CAR(args))) <= 0)
		errorcall(call, "third argument invalid\n");
	CAR(args) = coerceVector(CAR(args), REALSXP);
	if (k > *n) *n = k;
	args = CDR(args);

	if (!isNumeric(CAR(args)) || (k = LENGTH(CAR(args))) <= 0)
		errorcall(call, "fourth argument invalid\n");
	CAR(args) = coerceVector(CAR(args), REALSXP);
	if (k > *n) *n = k;
	args = CDR(args);
}

	/* segments(x0, y0, x1, y1, col, lty) */

SEXP do_segments(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sx0, sy0, sx1, sy1, col, lty;
	double *x0, *x1, *y0, *y1;
	int nx0, nx1, ny0, ny1;
	int i, n, ncol, colsave, nlty, ltysave;

	GCheckState(); 

	if(length(args) < 4) errorcall(call, "too few arguments\n");

	xypoints(call, args, &n);

	sx0 = CAR(args); nx0 = length(sx0); args = CDR(args);
	sy0 = CAR(args); ny0 = length(sy0); args = CDR(args);
	sx1 = CAR(args); nx1 = length(sx1); args = CDR(args);
	sy1 = CAR(args); ny1 = length(sy1); args = CDR(args);

	PROTECT(lty = FixupLty(GetPar("lty", args)));
	nlty = length(lty);

	PROTECT(col = FixupCol(GetPar("col", args)));
	ncol = LENGTH(col);

	x0 = REAL(sx0);
	y0 = REAL(sy0);
	x1 = REAL(sx1);
	y1 = REAL(sy1);
	colsave = GP->col;
	ltysave = GP->lty;
	GMode(1);
	for (i = 0; i < n; i++) {
		if (FINITE(x0[i%nx0]) && FINITE(y0[i%ny0])
		    && FINITE(x1[i%nx1]) && FINITE(y1[i%ny1])) {
			GP->col = INTEGER(col)[i % ncol];
			if(GP->col == NA_INTEGER) GP->col = colsave;
			GP->lty = INTEGER(lty)[i % nlty];
			GStartPath();
			GMoveTo(XMAP(x0[i % nx0]), YMAP(y0[i % ny0]));
			GLineTo(XMAP(x1[i % nx1]), YMAP(y1[i % ny1]));
			GEndPath();
		}
	}
	GMode(0);
	GP->col = colsave;
	GP->lty = ltysave;
	UNPROTECT(2);
	return R_NilValue;
}

	/* rect(xl, yb, xr, yt, col, border) */

SEXP do_rect(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sxl, sxr, syb, syt, col, lty, border;
	double *xl, *xr, *yb, *yt;
	int i, n, nxl, nxr, nyb, nyt;
	int ncol, nlty, nborder;
	int colsave, ltysave;

	GCheckState(); 

	if(length(args) < 4) errorcall(call, "too few arguments\n");
	xypoints(call, args, &n);

	sxl = CAR(args); nxl = length(sxl); args = CDR(args);
	syb = CAR(args); nyb = length(syb); args = CDR(args);
	sxr = CAR(args); nxr = length(sxr); args = CDR(args);
	syt = CAR(args); nyt = length(syt); args = CDR(args);

	PROTECT(col = FixupCol(GetPar("col", args)));
	ncol = LENGTH(col);

	PROTECT(border =  FixupCol(GetPar("border", args)));
	nborder = LENGTH(border);

	PROTECT(lty = FixupLty(GetPar("lty", args)));
	nlty = length(lty);

	xl = REAL(sxl);
	xr = REAL(sxr);
	yb = REAL(syb);
	yt = REAL(syt);

	ltysave = GP->lty;
	colsave = GP->col;
	GMode(1);
	for (i = 0; i < n; i++) {
		if (FINITE(xl[i%nxl]) && FINITE(yb[i%nyb])
		    && FINITE(xr[i%nxr]) && FINITE(yt[i%nyt]))
				GRect(XMAP(xl[i % nxl]), YMAP(yb[i % nyb]),
				      XMAP(xr[i % nxr]), YMAP(yt[i % nyt]),
					INTEGER(col)[i % ncol],
					INTEGER(border)[i % nborder]);
	}
	GMode(0);
	GP->col = colsave;
	GP->lty = ltysave;
	UNPROTECT(3);
	return R_NilValue;
}

	/* do_arrows(x0, y0, x1, y1, length, angle, code, col) */

SEXP do_arrows(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sx0, sx1, sy0, sy1, col, lty;
	double *x0, *x1, *y0, *y1;
	double hlength, angle;
	int code, i, n, nx0, nx1, ny0, ny1;
	int ncol, colsave, nlty, ltysave, xpd;

	GCheckState(); 

	if(length(args) < 4) errorcall(call, "too few arguments\n");
	xypoints(call, args, &n);

	sx0 = CAR(args); nx0 = length(sx0); args = CDR(args);
	sy0 = CAR(args); ny0 = length(sy0); args = CDR(args);
	sx1 = CAR(args); nx1 = length(sx1); args = CDR(args);
	sy1 = CAR(args); ny1 = length(sy1); args = CDR(args);

	hlength = asReal(GetPar("length", args));
	if (!FINITE(hlength) || hlength <= 0)
		errorcall(call, "invalid head length\n");

	angle = asReal(GetPar("angle", args));
	if (!FINITE(angle))
		errorcall(call, "invalid head angle\n");

	code = asInteger(GetPar("code", args));
	if (code == NA_INTEGER || code < 0 || code > 3)
		errorcall(call, "invalid arrow head specification\n");

	PROTECT(col = FixupCol(GetPar("col", args)));
	ncol = LENGTH(col);

	PROTECT(lty = FixupLty(GetPar("lty", args)));
	nlty = length(lty);

	xpd = asLogical(GetPar("xpd", args));
	if(xpd == NA_LOGICAL) xpd = GP->xpd;

	x0 = REAL(sx0);
	y0 = REAL(sy0);
	x1 = REAL(sx1);
	y1 = REAL(sy1);

	colsave = GP->col;
	ltysave = GP->lty;
	GMode(1);
	for (i = 0; i < n; i++) {
		if (FINITE(x0[i%nx0]) && FINITE(y0[i%ny0])
		    && FINITE(x1[i%nx1]) && FINITE(y1[i%ny1])) {
			GP->col = INTEGER(col)[i % ncol];
			if(GP->col == NA_INTEGER) GP->col = colsave;
			if(nlty == 0 || INTEGER(lty)[i % nlty] == NA_INTEGER)
				GP->lty = ltysave;
			else
				GP->lty = INTEGER(lty)[i % nlty];
		}
		GArrow(XMAP(x0[i % nx0]), YMAP(y0[i % ny0]),
		       XMAP(x1[i % nx1]), YMAP(y1[i % ny1]),
		       hlength, angle, code);
	}
	GMode(0);
	GP->col = colsave;
	GP->lty = ltysave;
	UNPROTECT(2);
	return R_NilValue;
}

	/* polygon(x, y, col, border) */

SEXP do_polygon(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sx, sy, col, border, lty;
	int colsave, ltysave, xpd, xpdsave;
	int nx, ny, ncol, nborder, nlty;
	double *work;
	char *vmax;

	GCheckState(); 

	if(length(args) < 2) errorcall(call, "too few arguments\n");

	if (!isNumeric(CAR(args)) || (nx = LENGTH(CAR(args))) <= 0)
		errorcall(call, "first argument invalid\n");
	sx = CAR(args) = coerceVector(CAR(args), REALSXP);
	args = CDR(args);

	if (!isNumeric(CAR(args)) || (ny = LENGTH(CAR(args))) <= 0)
		errorcall(call, "second argument invalid\n");
	sy = CAR(args) = coerceVector(CAR(args), REALSXP);
	args = CDR(args);

	if (ny != nx)
		errorcall(call, "x and y lengths differ in polygon");

	PROTECT(col = FixupCol(GetPar("col", args)));
	ncol = LENGTH(col);

	PROTECT(border = FixupCol(GetPar("border", args)));
	nborder = LENGTH(border);

	PROTECT(lty = FixupLty(GetPar("lty", args)));
	nlty = length(lty);

	xpd = asLogical(GetPar("xpd", args));
	if(xpd == NA_LOGICAL) xpd = GP->xpd;

	colsave = GP->col;
	ltysave = GP->lty;
	xpdsave = GP->xpd;
	GMode(1);
	vmax = vmaxget();
	work = (double*)R_alloc(2*nx, sizeof(double));
	if(INTEGER(lty)[0] == NA_INTEGER) GP->lty = ltysave;
	else GP->lty = INTEGER(lty)[0];
	GPolygon(nx, REAL(sx), REAL(sy), INTEGER(col)[0], INTEGER(border)[0], 1, work);
	vmaxset(vmax);
	GMode(0);
	GP->col = colsave;
	GP->lty = ltysave;
	GP->xpd = xpdsave;
	UNPROTECT(3);
	return R_NilValue;
}

SEXP do_text(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP sx, sy, sxy, txt, adj, cex, col, font;
	int i, n, ncex, ncol, nfont, ntxt;
	double adjx, adjy, cexsave;
	int colsave, fontsave, xpd, xpdsave;
	double *x, *y;
	double xx, yy;

	GCheckState(); 

	if(length(args) < 2) errorcall(call, "too few arguments\n");

	sxy = CAR(args);
	if (!isList(sxy) || length(sxy) < 2)
		errorcall(call, "invalid plotting structure\n");
	internalTypeCheck(call, sx = CAR(sxy), REALSXP);
	internalTypeCheck(call, sy = CADR(sxy), REALSXP);
	if (LENGTH(sx) != LENGTH(sy))
		error("x and y lengths differ for plot\n");
	n = LENGTH(sx);
	args = CDR(args);

	internalTypeCheck(call, txt = CAR(args), STRSXP);
	if (LENGTH(txt) <= 0)
		errorcall(call, "zero length \"text\" specified\n");
	args = CDR(args);

	PROTECT(cex = FixupCex(GetPar("cex", args)));
	ncex = LENGTH(cex);

	PROTECT(col = FixupCol(GetPar("col", args)));
	ncol = LENGTH(col);

	PROTECT(font = FixupFont(GetPar("font", args)));
	nfont = LENGTH(font);

	PROTECT(adj = GetPar("adj", args));
	if(isNull(adj) || (isNumeric(adj) && length(adj) == 0)) {
		adjx = GP->adj;
		adjy = GP->yCharOffset;
	}
	else if(isReal(adj)) {
		if(LENGTH(adj) == 1) {
			adjx = REAL(adj)[0];
			adjy = GP->yCharOffset;
		}
		else {
			adjx = REAL(adj)[0];
			adjy = REAL(adj)[1];
		}
	}
	else errorcall(call, "invalid adj value");

	xpd = asLogical(GetPar("xpd", args));
	if(xpd == NA_LOGICAL) xpd = 0;

	x = REAL(sx);
	y = REAL(sy);
	n = LENGTH(sx);
	ntxt = LENGTH(txt);

	cexsave = GP->cex;
	colsave = GP->col;
	fontsave = GP->font;
	xpdsave = GP->xpd;
	GP->xpd = xpd;

	GMode(1);
	for (i = 0; i < n; i++) {
		xx = xt(x[i % n]);
		yy = yt(y[i % n]);
		if (FINITE(xx) && FINITE(yy)) {
			if (ncol && INTEGER(col)[i % ncol] != NA_INTEGER)
				GP->col = INTEGER(col)[i % ncol];
			else GP->col = colsave;
			if(ncex && FINITE(REAL(cex)[i%ncex]))
				GP->cex = GP->cexbase * REAL(cex)[i % ncex];
			else GP->cex = GP->cexbase;
			if (nfont && INTEGER(font)[i % nfont] != NA_INTEGER)
				GP->font = INTEGER(font)[i % nfont];
			else GP->font = fontsave;
			GText(XMAP(xx), YMAP(yy), CHAR(STRING(txt)[i % ntxt]), adjx, adjy, 0.0);
		}
	}
	GMode(0);
	GP->col = colsave;
	GP->cex = cexsave;
	GP->font = fontsave;
	GP->xpd = xpdsave;
	UNPROTECT(4);
	return R_NilValue;
}

	/* mtext(text, side, line, outer, at = NULL) */

SEXP do_mtext(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP adj, cex, col, font, text;
	double line, at, adjx, adjy, adjsave, cexsave, xpdsave;
	int colsave, fontsave, side, outer; short int i;

	GCheckState(); 

	if(length(args) < 5) errorcall(call, "too few arguments");

	internalTypeCheck(call, text = CAR(args), STRSXP);
	if (LENGTH(text) <= 0)
		errorcall(call, "zero length \"text\" specified\n");
	args = CDR(args);

	side = asInteger(CAR(args));
	if(side < 1 || side > 4) errorcall(call, "invalid side value\n");
	args = CDR(args);

	line = asReal(CAR(args));
	if(!FINITE(line)) /* || line < 0.0 -- negative values make sense ! */
	  errorcall(call, "invalid line value\n");
	args = CDR(args);

	outer = asInteger(CAR(args));
	if(outer == NA_INTEGER) outer = 0;
	args = CDR(args);

	if (CAR(args) != R_NilValue && LENGTH(CAR(args)) > 0) {
		at = asReal(CAR(args));
		if(!FINITE(at)) errorcall(call, "invalid at value\n");
		args = CDR(args);
	}
	else {	/*-- default for at: middle of current side : */
		i = (side % 2) ? 0 : 2;
		at = 0.5*(GP->usr[i] + GP->usr[i+1]);
	}
	adjsave = GP->adj;
	cexsave = GP->cex;
	colsave = GP->col;
	fontsave = GP->font;
	xpdsave = GP->xpd;

	PROTECT(adj = GetPar("adj", args));
	if(isNull(adj) || (isNumeric(adj) && length(adj) == 0)) {
		adjx = GP->adj;
		adjy = GP->yCharOffset;
	}
	else if(isReal(adj)) {
		if(LENGTH(adj) == 1) {
			adjx = REAL(adj)[0];
			adjy = GP->yCharOffset;
		}
		else {
			adjx = REAL(adj)[0];
			adjy = REAL(adj)[1];
		}
	}
	else errorcall(call, "invalid adj value");

	/*======= now you should MAKE USE of adjx, adjy !!! =======*/

	PROTECT(cex = FixupCex(GetPar("cex", args)));
	if(FINITE(REAL(cex)[0])) GP->cex = GP->cexbase * REAL(cex)[0];
	else GP->cex = GP->cexbase;

	PROTECT(col = FixupCol(GetPar("col", args)));
	if(INTEGER(col)[0] != NA_INTEGER) GP->col = INTEGER(col)[0];

	PROTECT(font = FixupFont(GetPar("font", args)));
	if(INTEGER(font)[0] != NA_INTEGER) GP->font = INTEGER(font)[0];

	GP->adj = adjx;
	GP->xpd = 1;
	GMode(1);
	GMtext(CHAR(STRING(text)[0]), side, line, outer, at, 0);
	GMode(0);
	GP->cex = cexsave;
	GP->col = colsave;
	GP->font = fontsave;
	GP->xpd = xpdsave;
	GP->adj = adjsave;
	UNPROTECT(4);
	return R_NilValue;
}

	/* Title(main=NULL, sub=NULL, xlab=NULL, ylab=NULL, ...) */

SEXP do_title(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP main, cexmain, colmain, fontmain;
	SEXP xlab, ylab, cexlab, collab, fontlab;
	SEXP sub, cexsub, colsub, fontsub;
	int colsave, fontsave, xpdsave;
	double cexsave, x, y;

	GCheckState(); 

	if(length(args) < 4) errorcall(call, "too few arguments");

	main = sub = xlab = ylab = R_NilValue;

	if (CAR(args) != R_NilValue && LENGTH(CAR(args)) > 0)
		main = STRING(CAR(args))[0];
	args = CDR(args);

	if (CAR(args) != R_NilValue && LENGTH(CAR(args)) > 0)
		sub = STRING(CAR(args))[0];
	args = CDR(args);

	if (CAR(args) != R_NilValue && LENGTH(CAR(args)) > 0)
		xlab = STRING(CAR(args))[0];
	args = CDR(args);

	if (CAR(args) != R_NilValue && LENGTH(CAR(args)) > 0)
		ylab = STRING(CAR(args))[0];
	args = CDR(args);

	PROTECT(cexmain = FixupCex(GetPar("cex.main", args)));
	PROTECT(colmain = FixupCol(GetPar("col.main", args)));
	PROTECT(fontmain = FixupFont(GetPar("font.main", args)));
	PROTECT(cexlab = FixupCex(GetPar("cex.lab", args)));
	PROTECT(collab = FixupCol(GetPar("col.lab", args)));
	PROTECT(fontlab = FixupFont(GetPar("font.lab", args)));
	PROTECT(cexsub = FixupCex(GetPar("cex.sub", args)));
	PROTECT(colsub = FixupCol(GetPar("col.sub", args)));
	PROTECT(fontsub = FixupFont(GetPar("font.sub", args)));

	cexsave = GP->cex;
	colsave = GP->col;
	fontsave = GP->font;
	xpdsave = GP->xpd;
	GP->xpd = 1;

	x = fabs((GP->cra[1] * GP->mex) / GP->fig2dev.bx);
	y = fabs((GP->asp * GP->cra[1] * GP->mex)/GP->fig2dev.by);

	GMode(1);
	if(main != R_NilValue) {
		if(FINITE(REAL(cexmain)[0]))
			GP->cex = GP->cexbase * REAL(cexmain)[0];
		else GP->cex = GP->cexbase * GP->cexmain;
		GP->col = INTEGER(colmain)[0];
		if(GP->col == NA_INTEGER) GP->col = GP->colmain;
		GP->font = INTEGER(fontmain)[0];
		if(GP->font == NA_INTEGER) GP->font = GP->fontmain;
		GText(0.5*GP->plt[0]+0.5*GP->plt[1], 0.5*GP->plt[3]+0.5,
			CHAR(main), 0.5, 0.5, 0.0);
	}
	if(sub != R_NilValue) {
		if(FINITE(REAL(cexsub)[0]))
			GP->cex = GP->cexbase * REAL(cexsub)[0];
		else GP->cex = GP->cexbase * GP->cexsub;
		GP->col = INTEGER(colsub)[0];
		if(GP->col == NA_INTEGER) GP->col = GP->colsub;
		GP->font = INTEGER(fontsub)[0];
		if(GP->font == NA_INTEGER) GP->font = GP->fontsub;
		if(sub != R_NilValue)
			GMtext(CHAR(sub), 1, GP->mgp[0]+1.0, 0, 0.5*GP->usr[0]+0.5*GP->usr[1], 0);
	}
	if(xlab != R_NilValue || ylab != R_NilValue) {
		if(FINITE(REAL(cexlab)[0]))
			GP->cex = GP->cexbase * REAL(cexlab)[0];
		else GP->cex = GP->cexbase * GP->cexlab;
		GP->col = INTEGER(collab)[0];
		if(GP->col == NA_INTEGER) GP->col = GP->collab;
		GP->font = INTEGER(fontlab)[0];
		if(GP->font == NA_INTEGER) GP->font = GP->fontlab;
		if(xlab != R_NilValue)
			GMtext(CHAR(xlab), 1, GP->mgp[0], 0, 0.5*GP->usr[0]+0.5*GP->usr[1], 0);
		if(ylab != R_NilValue)
			GMtext(CHAR(ylab), 2, GP->mgp[0], 0, 0.5*GP->usr[2]+0.5*GP->usr[3], 0);
	}
	GMode(0);

	GP->cex = cexsave;
	GP->col = colsave;
	GP->font = fontsave;
	GP->xpd = xpdsave;
	UNPROTECT(9);
	return R_NilValue;
}

SEXP do_abline(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP a, b, h, v, col, lty;
	int i, ncol, nlines, nlty;
	double aa, bb;

	GCheckState(); 
	if(length(args) < 4) errorcall(call, "too few arguments\n");

	if((a = CAR(args)) != R_NilValue)
		CAR(args) = a = coerceVector(a, REALSXP);
	args = CDR(args);

	if((b = CAR(args)) != R_NilValue)
		CAR(args) = b = coerceVector(b, REALSXP);
	args = CDR(args);

	if((h = CAR(args)) != R_NilValue)
		CAR(args) = h = coerceVector(h, REALSXP);
	args = CDR(args);

	if((v = CAR(args)) != R_NilValue)
		CAR(args) = v = coerceVector(v, REALSXP);
	args = CDR(args);

	PROTECT(col = FixupCol(CAR(args)));
	ncol = LENGTH(col);
	args = CDR(args);

	PROTECT(lty = FixupLty(CAR(args)));
	nlty = length(lty);
	args = CDR(args);

	SavePars();
	ProcessInlinePars(args);
	
	nlines = 0;

	if (a != R_NilValue) {
		if (b == R_NilValue) {
			if (LENGTH(a) != 2)
				errorcall(call, "invalid a=, b= specification in \"abline\"\n");
			aa = REAL(a)[0];
			bb = REAL(a)[1];
		}
		else {
			aa = asReal(a);
			bb = asReal(b);
		}
		if (!FINITE(aa) || !FINITE(bb))
			errorcall(call, "\"a\" and \"b\" must be non-missing\n");
		GP->col = INTEGER(col)[i % ncol];
		if(nlty && INTEGER(lty)[i % nlty] != NA_INTEGER) GP->lty = INTEGER(lty)[i % nlty];
		else GP->lty = ltysave;
		GMode(1);
		GStartPath();
		GMoveTo(XMAP(GP->usr[0]), YMAP(aa + GP->usr[0] * bb));
		GLineTo(XMAP(GP->usr[1]), YMAP(aa + GP->usr[1] * bb));
		GEndPath();
		GMode(0);
		nlines++;
	}
	if (h != R_NilValue) {
		GMode(1);
		for (i = 0; i < LENGTH(h); i++) {
			GP->col = INTEGER(col)[i % ncol];
			if(nlty && INTEGER(lty)[i % nlty] != NA_INTEGER) GP->lty = INTEGER(lty)[i % nlty];
			else GP->lty = ltysave;
			aa = yt(REAL(h)[i]);
			if (FINITE(aa)) {
				GStartPath();
				GMoveTo(XMAP(GP->usr[0]), YMAP(aa));
				GLineTo(XMAP(GP->usr[1]), YMAP(aa));
				GEndPath();
			}
			nlines++;
		}
		GMode(0);
	}
	if (v != R_NilValue) {
		GMode(1);
		for (i = 0; i < LENGTH(v); i++) {
			GP->col = INTEGER(col)[i % ncol];
			if(nlty && INTEGER(lty)[i % nlty] != NA_INTEGER) GP->lty = INTEGER(lty)[i % nlty];
			else GP->lty = ltysave;
			aa = xt(REAL(v)[i]);
			if (FINITE(aa)) {
				GStartPath();
				GMoveTo(XMAP(aa), YMAP(GP->usr[2]));
				GLineTo(XMAP(aa), YMAP(GP->usr[3]));
				GEndPath();
			}
			nlines++;
		}
		GMode(0);
	}
	UNPROTECT(2);
	RestorePars();
	return R_NilValue;
}

SEXP do_box(SEXP call, SEXP op, SEXP args, SEXP env)
{
	int col, fg;
	GCheckState(); 
	SavePars();
	col = GP->col;
	fg = GP->fg;
	GP->col = NA_INTEGER;
	GP->fg = NA_INTEGER;
	GP->xpd = 1;
	ProcessInlinePars(args);
	if(GP->col == NA_REAL) fg = GP->col;
	if(GP->fg == NA_REAL) fg = GP->fg;
	GP->col = fg;
	GMode(1);
	GBox();
	GMode(0);
	RestorePars();
	return R_NilValue;
}


SEXP do_locator(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP x, y, nobs, ans;
	int i, n;
	
	GCheckState(); 

	checkArity(op, args);
	n = asInteger(CAR(args));
	if(n <= 0 || n == NA_INTEGER)
		error("invalid number of points in locator\n");
	PROTECT(x = allocVector(REALSXP, n));
	PROTECT(y = allocVector(REALSXP, n));
	PROTECT(nobs=allocVector(INTSXP,1));
	i = 0;
	
	GMode(2);
	while(i < n) {
		if(!GLocator(&(REAL(x)[i]), &(REAL(y)[i]), 1))
			break;
		i += 1;
	}
	GMode(0);
	INTEGER(nobs)[0] = i;
	while(i < n) {
		REAL(x)[i] = NA_REAL;
		REAL(y)[i] = NA_REAL;
		i += 1;
	}
	ans = allocList(3);
	UNPROTECT(3);
	CAR(ans) = x;
	CADR(ans) = y;
	CADDR(ans) = nobs;
	return ans;
}

#define THRESHOLD	0.25

#ifdef Macintosh
double hypot(double x, double y)
{
	return sqrt(x*x+y*y);
}
#endif

SEXP do_identify(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP ans, x, y, l, ind, pos;
	double xi, yi, xp, yp, d, dmin, offset;
	int i, imin, k, n;

	GCheckState(); 

	checkArity(op, args);
	x = CAR(args);
	y = CADR(args);
	l = CADDR(args);
	if(!isReal(x) || !isReal(y) || !isString(l))
		errorcall(call, "incorrect argument type\n");
	if(LENGTH(x) != LENGTH(y) || LENGTH(x) != LENGTH(l))
		errorcall(call, "different argument lengths\n");
	n = LENGTH(x);
	if(n <= 0) {
		R_Visible = 0;
		return NULL;
	}

	offset = xChartoInch(0.5);
	PROTECT(ind = allocVector(LGLSXP, n));
	PROTECT(pos = allocVector(INTSXP, n));
	for(i=0 ; i<n ; i++)
		LOGICAL(ind)[i] = 0;

	k = 0;
	GMode(2);
	while(k < n) {
		if(!GLocator(&xp, &yp, 0)) break;
		dmin = DBL_MAX;
		imin = -1;
		for(i=0 ; i<n ; i++) {
			xi = xt(REAL(x)[i]);
			yi = yt(REAL(y)[i]);
			if(!FINITE(xi) || !FINITE(yi)) continue;
			d = hypot(xFigtoInch(xp-XMAP(xi)), yFigtoInch(yp-YMAP(yi)));
			if(d < dmin) {
				imin = i;
				dmin = d;
			}
		}
		if(dmin > THRESHOLD)
			REprintf("warning: no point with %.2f inches\n", THRESHOLD);
		else if(LOGICAL(ind)[imin])
			REprintf("warning: nearest point already identified\n");
		else {
			LOGICAL(ind)[imin] = 1;
			xi = XMAP(xt(REAL(x)[imin]));
			yi = YMAP(yt(REAL(y)[imin]));
			if(fabs(xFigtoInch(xp-xi)) >= fabs(yFigtoInch(yp-yi))) {
				if(xp >= xi) {
					INTEGER(pos)[imin] = 4;
					xi = xi+xInchtoFig(offset);
					GText(xi, yi, CHAR(STRING(l)[imin]), 0.0, GP->yCharOffset, 0.0);
				}
				else {
					INTEGER(pos)[imin] = 2;
					xi = xi-xInchtoFig(offset);
					GText(xi, yi, CHAR(STRING(l)[imin]), 1.0, GP->yCharOffset, 0.0);
				}
			}
			else {
				if(yp >= yi) {
					INTEGER(pos)[imin] = 3;
					yi = yi+yInchtoFig(offset);
					GText(xi, yi, CHAR(STRING(l)[imin]), 0.5, 0.0, 0.0);
				}
				else {
					INTEGER(pos)[imin] = 1;
					yi = yi-yInchtoFig(offset);
					GText(xi, yi, CHAR(STRING(l)[imin]), 0.5, 1-(0.5-GP->yCharOffset), 0.0);
				}
			}
		}
	}
	GMode(0);
	ans = allocList(2);
	CAR(ans) = ind;
	CADR(ans) = pos;
	UNPROTECT(2);
	return ans;
}

	/* strwidth(str, units) */

SEXP do_strwidth(SEXP call, SEXP op, SEXP args, SEXP env)
{
	SEXP ans, str;
	int i, n, units;
	double cex, cexsave;

	/* GCheckState(); */

	checkArity(op, args);
	str = CAR(args);
	if(TYPEOF(str) != STRSXP)
		errorcall(call, "character first argument expected\n");
	args = CDR(args);

	if((units = asInteger(CAR(args))) == NA_INTEGER || units < 0)
		errorcall(call, "invalid units\n");
	args = CDR(args);

	if(isNull(CAR(args)))
		cex = GP->cex;
	else if(!FINITE(cex = asReal(CAR(args))) || cex <= 0.0)
		errorcall(call, "invalid cex value\n");

	n = LENGTH(str);
	ans = allocVector(REALSXP, n);
	cexsave = GP->cex;
	GP->cex = cex * GP->cexbase;
	for(i=0 ; i<n ; i++)
		REAL(ans)[i] = GStrWidth(CHAR(STRING(str)[i]), units);
	GP->cex = cexsave;
	return ans;
}

static int n;
static int *lptr;
static int *rptr;
static double *hght;
static double *xpos;
static double hang;
static double offset;
static SEXP *llabels;

static void drawdend(int node, double *x, double *y)
{
	double xl, xr, yl, yr;
	int k;

	*y = hght[node-1];

	k = lptr[node-1];
	if(k > 0) drawdend(k, &xl, &yl);
	else {
		xl = xpos[-k-1];
		if(hang >= 0) yl = *y - hang;
		else yl = 0;
		GText(XMAP(xl), YMAP(yl-offset), CHAR(llabels[-k-1]), 1.0, 0.3, 90.0);
	}
	k = rptr[node-1];
	if(k > 0) drawdend(k, &xr, &yr);
	else {
		xr = xpos[-k-1];
		if(hang >= 0) yr = *y - hang;
		else yr = 0;
		GText(XMAP(xr), YMAP(yr-offset), CHAR(llabels[-k-1]), 1.0, 0.3, 90.0);
	}
	GStartPath();
	GMoveTo(XMAP(xl), YMAP(yl));
	GLineTo(XMAP(xl), YMAP(*y));
	GLineTo(XMAP(xr), YMAP(*y));
	GLineTo(XMAP(xr), YMAP(yr));
	GEndPath();
	*x = 0.5 * (xl + xr);
}

SEXP do_dend(SEXP call, SEXP op, SEXP args, SEXP env)
{
	int xpdsave;
	double x, y, ypin;

	checkArity(op, args);

	GCheckState(); 

	n = asInteger(CAR(args));
	if(n == NA_INTEGER || n < 2)
		goto badargs;
	args = CDR(args);

	if(TYPEOF(CAR(args)) != INTSXP || length(CAR(args)) != 2*n)
		goto badargs;
	lptr = &(INTEGER(CAR(args))[0]);
	rptr = &(INTEGER(CAR(args))[n]);
	args = CDR(args);

	if(TYPEOF(CAR(args)) != REALSXP || length(CAR(args)) != n)
		goto badargs;
	hght = REAL(CAR(args));
	args = CDR(args);

	if(TYPEOF(CAR(args)) != REALSXP || length(CAR(args)) != n+1)
		goto badargs;
	xpos = REAL(CAR(args));
	args = CDR(args);

	hang = asReal(CAR(args));
	if(!FINITE(hang))
		goto badargs;
	args = CDR(args);

	if(TYPEOF(CAR(args)) != STRSXP || length(CAR(args)) != n+1)
		goto badargs;
	llabels = STRING(CAR(args));

	ypin = yNDCtoInch(DP->fig2dev.by / DP->ndc2dev.by) * (DP->plt[3] - DP->plt[2]);
	offset = GStrWidth("m", 3) * (GP->usr[3] - GP->usr[2]) / ypin;

	xpdsave = GP->xpd;
	GP->xpd = 1;
	GMode(1);
	drawdend(n, &x, &y);
	GMode(0);
	GP->xpd = xpdsave;

	return R_NilValue;

badargs:
	error("invalid dendrogram input\n");
}

SEXP do_saveplot(SEXP call, SEXP op, SEXP args, SEXP env)
{
	checkArity(op, args);

	GCheckState(); 

	if(!isString(CAR(args)) || length(CAR(args)) < 1 || *CHAR(STRING(CAR(args))[0]) == '\0')
		errorcall(call, "file name expected as argument\n");
	GSavePlot(CHAR(STRING(CAR(args))[0]));
	return R_NilValue;
}

SEXP do_printplot(SEXP call, SEXP op, SEXP args, SEXP env)
{
	checkArity(op, args);

	GCheckState(); 

	GPrintPlot();
	return R_NilValue;
}
/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996, 1997 Robert Gentleman and Ross Ihaka
 *
 *  This source code module:
 *  Copyright (C) 1997 Paul Murrell and Ross Ihaka
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

#include "Mathlib.h"
#include "Graphics.h"
#include "Defn.h"

		/* return maximum of two doubles */

static double max(double x, double y)
{
	if (x > y) return x;
	else return y;
}

		/* determine match between symbol name and string */

static int symbolMatch(SEXP expr, char *aString)
{
	return !strcmp(CHAR(PRINTNAME(expr)), aString);
}

		/* code to determine the ascii code corresponding */
		/* to an element of a mathematical expression */

static int hatAscii()
{
	return 94;
}

static int tildeAscii()
{
	return 126;
}

static int accentAscii(SEXP expr)
{
	int result = 0;

	if (symbolMatch(expr, "hat"))
		result = hatAscii();
	else if (symbolMatch(expr, "tilde"))
		result = tildeAscii();

	return result;
}

static int operatorAscii(SEXP expr)
{
	int result = 0;

	if (symbolMatch(expr, "sum"))
		result = 229;
	else if (symbolMatch(expr, "integral"))
		result = 242;
	else if (symbolMatch(expr, "product"))
		result = 213;

	return result;
}

static int groupOpenAscii()
{
	return 40;
}

static int groupCloseAscii()
{
	return 41;
}

static int commaAscii()
{
	return 44;
}

static int spaceAscii()
{
	return 32;
}

static int radicalAscii()
{
	return 214;
}

static int radicalExAscii()
{
	return 96;
}

static struct {
	char *name;
	int code;
} GreekTable[] = {

	"Alpha", 65,
	"Beta", 66,
	"Chi", 67,
	"Delta", 68,
	"Epsilon", 69,
	"Phi", 70,
	"Gamma", 71,
	"Eta", 72,
	"Iota", 73,
	"Phi1", 74,
	"Kappa", 75,
	"Lambda", 76,
	"Mu", 77,
	"Nu", 78,
	"Omicron", 79,
	"Pi", 80,
	"Theta", 81,
	"Rho", 82,
	"Sigma", 83,
	"Tau", 84,
	"Upsilon", 85,
	"sigma1", 86,
	"Omega", 87,
	"Xi", 88,
	"Psi", 89,
	"Zeta", 90,

	"alpha", 97,
	"beta", 98,
	"chi", 99,
	"delta", 100,
	"epsilon", 101,
	"phi", 102,
	"gamma", 103,
	"eta", 104,
	"iota", 105,
	"phi1", 106,
	"kappa", 107,
	"lambda", 108,
	"mu", 109,
	"nu", 110,
	"omicron", 111,
	"pi", 112,
	"theta", 113,
	"rho", 114,
	"sigma", 115,
	"tau", 116,
	"upsilon", 117,
	"omega1", 118,
	"omega", 119,
	"xi", 120,
	"psi", 121,
	"zeta", 122,

	NULL, 0,
};

static int greekAscii(SEXP expr)
{
	int i;

	for (i = 0; GreekTable[i].code; i++)
		if (symbolMatch(expr, GreekTable[i].name))
			return GreekTable[i].code;
	return 0;
}

	/* initialisation code for mathematical notation */

static double ratioScale = 0.8;
static double scriptScale = 0.65;
static int ratioDepth = 0;
static int metricUnit = 3;

static SEXP plusSymbol;
static SEXP minusSymbol;
static SEXP timesSymbol;
static SEXP divideSymbol;
static SEXP equalSymbol;
static SEXP superSymbol;
static SEXP subSymbol;
static SEXP groupSymbol;

static void initFormulaSymbols()
{
	plusSymbol = install("+");
	minusSymbol = install("-");
	timesSymbol = install("*");
	divideSymbol = install("/");
	equalSymbol = install("==");
	superSymbol = install("^");
	subSymbol = install("[");
	groupSymbol = install("(");
}

	/* code to determine the nature of an expression */

static int formulaExpression(SEXP expr)
{
	return (TYPEOF(expr) == LANGSXP);
}

static int symbolAtom(SEXP expr)
{
	return (TYPEOF(expr) == SYMSXP);
}

static int numberAtom(SEXP expr)
{
	return (TYPEOF(expr) == REALSXP) ||
	    (TYPEOF(expr) == INTSXP) ||
	    (TYPEOF(expr) == CPLXSXP);
}

static int stringAtom(SEXP expr)
{
	return (TYPEOF(expr) == STRSXP);
}

static int binAtom(SEXP expr)
{
	int result = symbolAtom(expr) &&
	((expr == plusSymbol) ||
	 (expr == minusSymbol) ||
	 (expr == timesSymbol) ||
	 (expr == divideSymbol));
	return result;
}

static int relAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    (expr == equalSymbol);
}

static int multiplicationOperator(SEXP expr)
{
	return binAtom(expr) &&
	    (expr == timesSymbol);
}

static int superAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    (expr == superSymbol);
}

static int subAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    (expr == subSymbol);
}

static int hatAtom(SEXP expr)
{
	return symbolMatch(expr, "hat");
}

static int barAtom(SEXP expr)
{
	return symbolMatch(expr, "bar");
}

static int accentAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    (hatAtom(expr) || barAtom(expr) ||
	     symbolMatch(expr, "tilde"));
}

static int fractionAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    symbolMatch(expr, "over");
}

static int groupAtom(SEXP expr)
{
	return symbolAtom(expr) && (expr == groupSymbol);
}

static int operatorAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    (symbolMatch(expr, "sum") ||
	     symbolMatch(expr, "integral") ||
	     symbolMatch(expr, "product"));
}

static int radicalAtom(SEXP expr)
{
	return symbolAtom(expr) &&
	    symbolMatch(expr, "root");
}

static int absAtom(SEXP expr)
{
	return symbolAtom(expr) && symbolMatch(expr, "abs");
}

static int curlyAtom(SEXP expr)
{
	return symbolAtom(expr) && symbolMatch(expr, "{");
}

static int greekSymbol(SEXP expr)
{
	int i;
	if (symbolAtom(expr)) {
		for (i = 0; GreekTable[i].code; i++)
			if (symbolMatch(expr, GreekTable[i].name))
				return 1;
	}
	return 0;
}

		/* code to determine a font from the */
		/* nature of the expression */

static int atomFontFace(SEXP expr)
{
	int fontFace = 1;
	if (symbolAtom(expr)) {
		if (greekSymbol(expr) ||
		    binAtom(expr) ||
		    relAtom(expr) ||
		    groupAtom(expr) ||
		    operatorAtom(expr) ||
		    radicalAtom(expr))
			fontFace = 5;
		else
#ifdef OLD
			fontFace = 3;
#else
			fontFace = 1;
#endif
	}
	return fontFace;
}

	/* code to determine the gap between */
	/* elements of the expression */

	/* a forward declaration */

static double fontHeight();

static int innerExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return fractionAtom(CAR(expr));
	else
		return 0;
}

static int ordAtom(SEXP expr);

static int ordAccentExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return (accentAtom(CAR(expr)) && ordAtom(CADR(expr)));
	else
		return 0;
}

static int ordSuperExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return (superAtom(CAR(expr)) && ordAtom(CADR(expr)));
	else
		return 0;
}

static int ordSubExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return (subAtom(CAR(expr)) && ordAtom(CADR(expr)));
	else
		return 0;
}

static int ordAtom(SEXP expr)
{
	return (symbolAtom(expr) ||
		numberAtom(expr) ||
		ordAccentExpression(expr) ||
		ordSuperExpression(expr) ||
		ordSubExpression(expr));
}

static int groupExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return groupAtom(CAR(expr));
	else
		return 0;
}

static int operatorExpression(SEXP expr)
{
	if (formulaExpression(expr))
		return operatorAtom(CAR(expr));
	else
		return 0;
}

static double elementGap(SEXP expr1, SEXP expr2)
{
	double gap = 0;
	if ((ordAtom(expr1) && operatorExpression(expr2)) ||
	    (operatorExpression(expr1) && ordAtom(expr2)) ||
	    (operatorExpression(expr1) && operatorExpression(expr2)))
		gap = 0.1;
	else if ((binAtom(expr1) &&
		  (ordAtom(expr2) ||
		   operatorExpression(expr2) ||
		   groupExpression(expr2) ||
		   innerExpression(expr2))) ||
		 ((ordAtom(expr1) ||
		   operatorExpression(expr1) ||
		   groupExpression(expr1) ||
		   innerExpression(expr1)) &&
		  binAtom(expr2)))
		gap = 0.2;
	else if ((relAtom(expr1) &&
		  (ordAtom(expr2) ||
		   operatorExpression(expr2) ||
		   groupExpression(expr2) ||
		   innerExpression(expr2))) ||
		 ((ordAtom(expr1) ||
		   operatorExpression(expr1) ||
		   groupExpression(expr1) ||
		   innerExpression(expr1)) &&
		  relAtom(expr2)))
		gap = 0.3;
	else if (groupExpression(expr1) && operatorExpression(expr2))
		gap = 0.1;
	else if ((innerExpression(expr1) &&
		  (ordAtom(expr2) ||
		   operatorExpression(expr2) ||
		   groupExpression(expr2) ||
		   innerExpression(expr2))) ||
		 ((ordAtom(expr1) ||
		   operatorExpression(expr1) ||
		   groupExpression(expr1) ||
		   innerExpression(expr1)) &&
		  innerExpression(expr2)))
		gap = 0.1;
	return gap * fontHeight();
}

	/* some forward declarations */

typedef struct {
	double height;
	double depth;
	double width;
} BBOX;

static double bboxHeight(BBOX bbox)
{
	return bbox.height;
}
static double bboxDepth(BBOX bbox)
{
	return bbox.depth;
}
static double bboxWidth(BBOX bbox)
{
	return bbox.width;
}
static BBOX asciiBBox(int ascii);
static BBOX elementBBox(SEXP expr);


	/* code to determine superscript offsets, etc ... */

	/* these "twiddle factors" are given as  */
	/* proportions of the current font height */
	/* NOTE that metric information is obtained */
	/* in INCHES so that the information will */
	/* be useful for rotated math.text */


static float SuperDrop = 0.3;
static float Superscript = 0.3;
static float SubDrop = 0.1;
static float Subscript = -0.3;
static float LineWidth = 0.05;

static float CustomAccentGap = 0.2;
static float CustomHatHeight = 0.3;
static float CustomRadicalWidth = 0.6;
static float CustomRadicalSpace = 0.1;
static float CustomRadicalGap = 0.2;
static float AbsSpace = 0.2;

static double fontHeight()
{
	double height, depth, width;
	GMetricInfo(0, &height, &depth, &width, metricUnit);
	return height + depth;
}

static double xHeight()
{
	int x = 'x';
	double xheight, depth, width;
	GMetricInfo(x, &xheight, &depth, &width, metricUnit);
	return xheight;
}

static double axisHeight()
{
	int plus = '+';
	double plusHeight, depth, width;
	GMetricInfo(plus, &plusHeight, &depth, &width, metricUnit);
	return 0.5 * plusHeight;
}

static double superscriptDrop()
{
	return SuperDrop * fontHeight();
}
static double subscriptDrop()
{
	return SubDrop * fontHeight();
}
static double superscript()
{
	return Superscript * fontHeight();
}
static double subscript()
{
	return Subscript * fontHeight();
}
static double lineWidth()
{
	return LineWidth * fontHeight();
}
static double numeratorShift()
{
	return axisHeight() + 3 * lineWidth();
}
static double denominatorShift()
{
	return axisHeight() - 3 * lineWidth() - xHeight();
}
static double radicalDrop()
{
	return 1.25 * lineWidth();
}
static double customAccentGap()
{
	return CustomAccentGap * fontHeight();
}
static double customHatHeight()
{
	return CustomHatHeight * fontHeight();
}
static double customRadicalWidth()
{
	return CustomRadicalWidth * fontHeight();
}
static double customRadicalSpace()
{
	return CustomRadicalSpace * fontHeight();
}
static double customRadicalGap()
{
	return CustomRadicalGap * fontHeight();
}
static double absSpace()
{
	return AbsSpace * fontHeight();
}


	/* should only be called when font=5 (symbol) */

static double radicalExWidth()
{
	int radicalEx = 96;
	double height, depth, REWidth;
	GMetricInfo(radicalEx, &height, &depth, &REWidth, metricUnit);
	return REWidth;
}

static double superscriptShift(SEXP body, SEXP sup)
{
	BBOX bodyBBox = elementBBox(body);
	BBOX superscriptBBox;
	float cexSaved = GP->cex;
	double temp1 = bboxHeight(bodyBBox) - superscriptDrop();
	double temp2 = superscript();
	double temp3;

	GP->cex = GP->cex * scriptScale;
	superscriptBBox = elementBBox(sup);
	GP->cex = cexSaved;
	temp3 = bboxDepth(superscriptBBox) + 0.25 * xHeight();

	return max(temp1, max(temp2, temp3));
}

static double subscriptShift(SEXP body, SEXP sub, int subOnly)
{
	BBOX bodyBBox = elementBBox(body);
	BBOX subscriptBBox;
	float cexSaved = GP->cex;
	double temp1 = bboxDepth(bodyBBox) + subscriptDrop();
	double temp2 = subscript();
	double temp3;

	GP->cex = GP->cex * scriptScale;
	subscriptBBox = elementBBox(sub);
	GP->cex = cexSaved;
	temp3 = bboxHeight(subscriptBBox) - (4 * xHeight() / 5);

	if (subOnly)
		return max(temp1, max(temp2, temp3));
	else
		return max(temp1, temp2);
}

static void supsubShift(SEXP body, SEXP sup, SEXP sub,
			double *supShift, double *subShift)
{
	BBOX superscriptBBox, subscriptBBox;
	float cexSaved = GP->cex;
	double temp1, temp2;

	GP->cex = GP->cex * scriptScale;
	superscriptBBox = elementBBox(sup);
	subscriptBBox = elementBBox(sub);
	GP->cex = cexSaved;
	*supShift = superscriptShift(body, sup);
	*subShift = subscriptShift(body, sub, 0);

	temp1 = (*supShift - bboxDepth(superscriptBBox)) -
	    (bboxHeight(subscriptBBox) - *subShift);
	if (temp1 < (4 * lineWidth()))
		*subShift = bboxHeight(subscriptBBox) -
		    *supShift +
		    bboxDepth(superscriptBBox) +
		    (4 * lineWidth());

	temp2 = (4 * xHeight() / 5) -
	    (*supShift - bboxDepth(superscriptBBox));
	if (temp2 > 0) {
		*supShift = *supShift + temp2;
		*subShift = *subShift - temp2;
	}
}

static double accentVShift(SEXP body)
{
	double temp = xHeight();
	double bodyHeight = bboxHeight(elementBBox(body));

	if (bodyHeight > temp)
		return bodyHeight - temp;
	else
		return 0;
}

static double accentHShift(SEXP body, SEXP accent)
{
	return (bboxWidth(elementBBox(body)) -
		bboxWidth(elementBBox(accent))) / 2;
}

static double numeratorVShift(SEXP num)
{
	BBOX numBBox;
	float cexSaved = GP->cex;
	double theShift, theClearance, minClearance;

	GP->cex = GP->cex * scriptScale;
	numBBox = elementBBox(num);
	GP->cex = cexSaved;

	theShift = numeratorShift();
	theClearance = numeratorShift() - bboxDepth(numBBox) -
	    (axisHeight() + 0.5 * lineWidth());
	minClearance = 3 * lineWidth();

	if (theClearance < minClearance)
		theShift += minClearance - theClearance;

	return theShift;
}

/* RATIO */
static double denominatorVShift(SEXP denom)
{
	BBOX denomBBox;
	float cexSaved = GP->cex;
	double theShift, theClearance, minClearance;

#ifdef OLD
	GP->cex = GP->cex * scriptScale;
#else
	GP->cex = GP->cex * ratioScale;
#endif
	denomBBox = elementBBox(denom);
#ifdef OLD
	GP->cex = cexSaved;
#else
	GP->cex = cexSaved;
#endif

	theShift = denominatorShift();
	theClearance = axisHeight() - 0.5 * lineWidth() -
	    (bboxHeight(denomBBox) - denominatorShift());
	minClearance = 3 * lineWidth();

	if (theClearance < minClearance)
		theShift += minClearance - theClearance;

	return theShift;
}

/* RATIO */
static double fractionWidth(SEXP num, SEXP denom)
{
	BBOX numBBox;
	BBOX denomBBox;
	float cexSaved = GP->cex;
	double temp1, temp2;

#ifdef OLD
	GP->cex = GP->cex * scriptScale;
#else
	GP->cex = GP->cex * ratioScale;
#endif
	numBBox = elementBBox(num);
	denomBBox = elementBBox(denom);
#ifdef OLD
	GP->cex = cexSaved;
#else
	GP->cex = cexSaved;
#endif
	temp1 = bboxWidth(numBBox);
	temp2 = bboxWidth(denomBBox);

	return max(temp1, temp2);
}

/* RATIO */
static void numdenomHShift(SEXP num, SEXP denom,
			   double *numShift, double *denomShift)
{
	BBOX numBBox;
	BBOX denomBBox;
	float cexSaved = GP->cex;
	double temp1, temp2;

#ifdef OLD
	GP->cex = GP->cex * scriptScale;
#else
	GP->cex = GP->cex * ratioScale;
#endif
	numBBox = elementBBox(num);
	denomBBox = elementBBox(denom);
#ifdef OLD
	GP->cex = cexSaved;
#else
	GP->cex = cexSaved;
#endif
	temp1 = bboxWidth(numBBox);
	temp2 = bboxWidth(denomBBox);

	if (temp1 > temp2) {
		*numShift = 0;
		*denomShift = (temp1 - temp2) / 2;
	}
	else {
		*numShift = (temp2 - temp1) / 2;
		*denomShift = 0;
	}
}

static int normalRadical(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	BBOX radicalBBox = asciiBBox(radicalAscii());

	return ((bboxHeight(bodyBBox) + radicalDrop())
		<= bboxHeight(radicalBBox)) &&
	    (bboxDepth(bodyBBox) <= bboxDepth(radicalBBox));
}

static double radicalVShift(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	return bboxHeight(bodyBBox) + radicalDrop();
}


	/* code to generate bounding boxes */


static BBOX makeBBox(double height, double depth, double width)
{
	BBOX bbox;
	bbox.height = height;
	bbox.depth = depth;
	bbox.width = width;
	return bbox;
}

static BBOX nullBBox()
{
	return makeBBox(0.0, 0.0, 0.0);
}

static BBOX makeBBoxFromChar(int chr)
{
	double height, depth, width;
	GMetricInfo(chr, &height, &depth, &width, metricUnit);
	return makeBBox(height, depth, width);
}

static BBOX shiftBBox(BBOX bbox, double shiftV)
{
	return makeBBox(bboxHeight(bbox) + shiftV,
			bboxDepth(bbox) - shiftV,
			bboxWidth(bbox));
}

static BBOX combineBBoxes(BBOX bbox1, BBOX bbox2)
{
	return makeBBox(max(bboxHeight(bbox1), bboxHeight(bbox2)),
			max(bboxDepth(bbox1), bboxDepth(bbox2)),
			bboxWidth(bbox1) + bboxWidth(bbox2));
}

static BBOX combineAlignedBBoxes(BBOX bbox1, BBOX bbox2)
{
	return makeBBox(max(bboxHeight(bbox1), bboxHeight(bbox2)),
			max(bboxDepth(bbox1), bboxDepth(bbox2)),
			max(bboxWidth(bbox1), bboxWidth(bbox2)));
}

static BBOX gapBBox(SEXP element1, SEXP element2)
{
	return makeBBox(0, 0, elementGap(element1, element2));
}

static BBOX smallgapBBox(SEXP element1, SEXP element2)
{
	return makeBBox(0, 0, 0.5 * elementGap(element1, element2));
}

	/* NOTE that i assume that all symbols which have */
	/* been converted to ascii are in the symbol font */

static BBOX asciiBBox(int ascii)
{
	if ((ascii == hatAscii()) || (ascii == tildeAscii()))
		GP->font = 1;
	else
		GP->font = 5;
	return makeBBoxFromChar(ascii);
}

static BBOX charBBox(char *str, SEXP expr)
{
	BBOX resultBBox = nullBBox();
	int i;

	GP->font = atomFontFace(expr);
	for (i = 0; i < strlen(str); i++)
		resultBBox = combineBBoxes(resultBBox, makeBBoxFromChar(str[i]));

	return resultBBox;
}

static BBOX symbolBBox(SEXP expr)
{
	if (greekSymbol(expr))
		return asciiBBox(greekAscii(expr));
	else
		return charBBox(CHAR(PRINTNAME(expr)), expr);
}

static BBOX numberBBox(SEXP expr)
{
	return charBBox(CHAR(asChar(expr)), expr);
}

static BBOX stringBBox(SEXP expr)
{
	return charBBox(CHAR(STRING(expr)[0]), expr);
}

static BBOX atomBBox(SEXP expr)
{
	if (symbolAtom(expr))
		return symbolBBox(expr);
	else if (numberAtom(expr))
		return numberBBox(expr);
	else if (stringAtom(expr))
		return stringBBox(expr);
}

static BBOX binBBox(SEXP expr)
{
	SEXP operator, operand1, operand2;
	BBOX middleBBox;

	operator = CAR(expr);
	operand1 = CADR(expr);
	if(length(expr) == 3) {
		operand2 = CADDR(expr);
		if (multiplicationOperator(operator))
			middleBBox = gapBBox(operand1, operand2);
		else
			middleBBox = combineBBoxes(gapBBox(operand1, operator),
					combineBBoxes(atomBBox(operator),
					   gapBBox(operator, operand2)));

		return combineBBoxes(elementBBox(operand1),
				     combineBBoxes(middleBBox,
						   elementBBox(operand2)));
	}
	else if(length(expr) == 2) {
		middleBBox = combineBBoxes(atomBBox(operator),
				   smallgapBBox(operator, operand1));
		return combineBBoxes(middleBBox, elementBBox(operand1));
	}
	else error("invalid formula\n");
}

static BBOX supsubBBox(SEXP body, SEXP superscript, SEXP subscript);

static BBOX superscriptBBox(SEXP superscript)
{
	BBOX result;
	float cexSaved = GP->cex;

	GP->cex = GP->cex * scriptScale;
	result = elementBBox(superscript);
	GP->cex = cexSaved;

	return result;
}

static BBOX supBBox(SEXP expr)
{
	BBOX result;
	SEXP body = CADR(expr);
	SEXP superscript = CADDR(expr);
	double supShift;
	int supsub = 0;

	if (formulaExpression(body))
		if (subAtom(CAR(body)))
			supsub = 1;

	if (supsub)
		return supsubBBox(CADR(body), superscript, (CADDR(body)));
	else
		return combineBBoxes(elementBBox(body),
				  shiftBBox(superscriptBBox(superscript),
				   superscriptShift(body, superscript)));
}

static BBOX subscriptBBox(SEXP subscript)
{
	BBOX result;
	float cexSaved = GP->cex;

	GP->cex = GP->cex * scriptScale;
	result = elementBBox(subscript);
	GP->cex = cexSaved;

	return result;
}

static BBOX subBBox(SEXP expr)
{
	SEXP body = CADR(expr);
	SEXP subscript = CADDR(expr);

	return combineBBoxes(elementBBox(body),
			     shiftBBox(subscriptBBox(subscript),
				    subscriptShift(body, subscript, 1)));
}

static BBOX supsubBBox(SEXP body, SEXP superscript, SEXP subscript)
{
	double supShift, subShift;
	supsubShift(body, superscript, subscript, &supShift, &subShift);

	return combineBBoxes(elementBBox(body),
			     combineAlignedBBoxes(
		       shiftBBox(superscriptBBox(superscript), supShift),
			 shiftBBox(subscriptBBox(subscript), subShift)));
}

static BBOX hatBBox(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	return combineAlignedBBoxes(bodyBBox,
				    makeBBox(bboxHeight(bodyBBox) +
					     customAccentGap() +
					     customHatHeight(), 0, 0));
}

static BBOX barBBox(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	return combineAlignedBBoxes(bodyBBox,
				    makeBBox(bboxHeight(bodyBBox) +
					     customAccentGap(), 0, 0));
}

static BBOX accentBBox(SEXP expr)
{
	SEXP accent = CAR(expr);
	SEXP body = CADR(expr);

	if (hatAtom(accent))
		return hatBBox(body);
	else if (barAtom(accent))
		return barBBox(body);
	else
		return combineAlignedBBoxes(
						   elementBBox(body),
		combineBBoxes(makeBBox(accentHShift(body, accent), 0, 0),
			      shiftBBox(asciiBBox(accentAscii(accent)),
					accentVShift(body))));
}

/* RATIO */
static BBOX fractionBBox(SEXP expr)
{
	SEXP numerator = CADR(expr);
	SEXP denominator = CADDR(expr);
	BBOX numBBox, denomBBox;
	double numHShift, denomHShift;
	float cexSaved = GP->cex;

#ifdef OLD
	GP->cex = GP->cex * scriptScale;
#else
	GP->cex = GP->cex * ratioScale;
#endif
	numBBox = elementBBox(numerator);
	denomBBox = elementBBox(denominator);
#ifdef OLD
	GP->cex = cexSaved;
#else
	GP->cex = cexSaved;
#endif
	numdenomHShift(numerator, denominator, &numHShift, &denomHShift);

	return combineAlignedBBoxes(
	     shiftBBox(combineBBoxes(makeBBox(numHShift, 0, 0), numBBox),
		       numeratorVShift(numerator)),
	 shiftBBox(combineBBoxes(makeBBox(denomHShift, 0, 0), denomBBox),
		   -denominatorVShift(denominator)));
}

static BBOX groupBBox(SEXP expr)
{
	return combineBBoxes(asciiBBox(groupOpenAscii()),
			     combineBBoxes(elementBBox(CADR(expr)),
					   asciiBBox(groupCloseAscii())));
}

static BBOX operatorBBox(SEXP expr)
{
	return combineBBoxes(asciiBBox(operatorAscii(CAR(expr))),
			     elementBBox(CADR(expr)));
}

static BBOX customRadicalBBox(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	return combineBBoxes(makeBBox(bboxHeight(bodyBBox) + customRadicalGap(),
				      0, customRadicalWidth()),
		      combineBBoxes(makeBBox(0, 0, customRadicalSpace()),
				    bodyBBox));
}

static BBOX radicalBBox(SEXP expr)
{
	SEXP body = CADR(expr);
	return customRadicalBBox(body);
}

static BBOX absBBox(SEXP expr)
{
	SEXP body = CADR(expr);
	return combineBBoxes(makeBBox(0, 0, absSpace()),
			     combineBBoxes(elementBBox(body),
					   makeBBox(0, 0, absSpace())));
}

static BBOX expressionBBox(SEXP expr)
{
	int i;
	int numParams = length(expr) - 1;
	BBOX resultBBox = elementBBox(CAR(expr));

	expr = CDR(expr);
	resultBBox = combineBBoxes(resultBBox, asciiBBox(groupOpenAscii()));
	for (i = 0; i < numParams; i++) {
		resultBBox = combineBBoxes(resultBBox, elementBBox(CAR(expr)));
		expr = CDR(expr);
		if (i < numParams - 1)
			resultBBox = combineBBoxes(resultBBox,
				   combineBBoxes(asciiBBox(commaAscii()),
					       asciiBBox(spaceAscii())));
	}
	return combineBBoxes(resultBBox, asciiBBox(groupCloseAscii()));
}

static BBOX curlyBBox(SEXP expr)
{
	return expressionBBox(CADR(expr));
}

static BBOX formulaBBox(SEXP expr)
{
	SEXP head = CAR(expr);

	if (binAtom(head))
		return binBBox(expr);
	else if (superAtom(head))
		return supBBox(expr);
	else if (subAtom(head))
		return subBBox(expr);
	else if (accentAtom(head))
		return accentBBox(expr);
	else if (fractionAtom(head))
		return fractionBBox(expr);
	else if (groupAtom(head))
		return groupBBox(expr);
	else if (operatorAtom(head))
		return operatorBBox(expr);
	else if (radicalAtom(head))
		return radicalBBox(expr);
	else if (absAtom(head))
		return absBBox(expr);
	else if (curlyAtom(head))
		return curlyBBox(expr);
	else
		return expressionBBox(expr);
}

static BBOX elementBBox(SEXP expr)
{
	if (formulaExpression(expr))
		return formulaBBox(expr);
	else
		return atomBBox(expr);
}

		/*
		   // code to draw mathematical notation
		 */

static double referenceX;
static double referenceY;
static double currentX;
static double currentY;
static double currentAngle;
static double cosAngle;
static double sinAngle;

		/* 
		   // convert currentX and currentY from inches and 0 angle
		   // to figure units and currentAngle
		 */
static double convertedX()
{
	double rotatedX = referenceX +
	(currentX - referenceX) * cosAngle -
	(currentY - referenceY) * sinAngle;
	return xInchtoFig(rotatedX);
}

static double convertedY()
{
	double rotatedY = referenceY +
	(currentY - referenceY) * cosAngle +
	(currentX - referenceX) * sinAngle;
	return yInchtoFig(rotatedY);
}

static void moveAcross(double xamount)
{
	currentX += xamount;
}

static void moveUp(double yamount)
{
	currentY += yamount;
}

static void moveTo(double x, double y)
{
	currentX = x;
	currentY = y;
}

static void drawGap(SEXP element1, SEXP element2)
{
	moveAcross(elementGap(element1, element2));
}

static void drawSmallGap(SEXP element1, SEXP element2)
{
	moveAcross(0.5 * elementGap(element1, element2));
}

static void drawAscii(int ascii)
{
	char asciiStr[2];

	if ((ascii == hatAscii()) || (ascii == tildeAscii()))
		GP->font = 1;
	else
		GP->font = 5;
	asciiStr[0] = ascii;
	asciiStr[1] = '\0';
	GText(convertedX(), convertedY(), asciiStr, 0.0, 0.0, currentAngle);
	moveAcross(GStrWidth(asciiStr, metricUnit));
}

static void drawChar(char *str, SEXP expr)
{
	GP->font = atomFontFace(expr);
	GText(convertedX(), convertedY(), str, 0.0, 0.0, currentAngle);
	moveAcross(GStrWidth(str, metricUnit));
}

static void drawSymbol(SEXP expr)
{
	if (greekSymbol(expr))
		drawAscii(greekAscii(expr));
	else
		drawChar(CHAR(PRINTNAME(expr)), expr);
}

static void drawNumber(SEXP expr)
{
	drawChar(CHAR(asChar(expr)), expr);
}

static void drawString(SEXP expr)
{
	drawChar(CHAR(STRING(expr)[0]), expr);
}

static void drawAtom(SEXP expr)
{
	if (symbolAtom(expr))
		drawSymbol(expr);
	else if (numberAtom(expr))
		drawNumber(expr);
	else if (stringAtom(expr))
		drawString(expr);
}

static void drawElement(SEXP expr);

static void drawBin(SEXP expr)
{
	SEXP operator, operand1, operand2;

	operator = CAR(expr);
	operand1 = CADR(expr);
	if(length(expr) == 3) {
		operand2 = CADDR(expr);
		drawElement(operand1);
		if (multiplicationOperator(operator))
			drawGap(operand1, operand2);
		else {
			drawGap(operand1, operator);
			drawAtom(operator);
			drawGap(operator, operand2);
		}
		drawElement(operand2);
	}
	else {
		drawAtom(operator);
		drawSmallGap(operator, operand1);
		drawElement(operand1);
	}
}

static void drawScriptElement(SEXP expr)
{
	float cexSaved = GP->cex;

	GP->cex = GP->cex * scriptScale;
	drawElement(expr);
	GP->cex = cexSaved;
}

static void drawRatioElement(SEXP expr)
{
	float cexSaved = GP->cex;
	GP->cex = GP->cex * ratioScale;
	drawElement(expr);
	GP->cex = cexSaved;
}

static void drawSupSub(SEXP body, SEXP superscript, SEXP subscript);

static void drawSuper(SEXP expr)
{
	SEXP body = CADR(expr);
	SEXP superscript = CADDR(expr);
	double supShift = superscriptShift(body, superscript);
	int supsub = 0;

	if (formulaExpression(body))
		if (subAtom(CAR(body)))
			supsub = 1;

	if (supsub)
		drawSupSub(CADR(body), superscript, CADDR(body));
	else {
		drawElement(body);
		moveUp(supShift);
		drawScriptElement(superscript);
		moveUp(-supShift);
	}
}

static void drawSub(SEXP expr)
{
	SEXP body = CADR(expr);
	SEXP subscript = CADDR(expr);
	double subShift = subscriptShift(body, subscript, 1);

	drawElement(body);
	moveUp(-subShift);
	drawScriptElement(subscript);
	moveUp(subShift);
}

static void drawSupSub(SEXP body, SEXP superscript, SEXP subscript)
{
	double supShift, subShift;
	double savedX, savedY;
	BBOX supBBox = elementBBox(superscript);
	BBOX subBBox = elementBBox(subscript);

	supsubShift(body, superscript, subscript, &supShift, &subShift);
	drawElement(body);
	savedX = currentX;
	savedY = currentY;
	moveUp(supShift);
	drawScriptElement(superscript);
	moveTo(savedX, savedY);
	moveUp(-subShift);
	drawScriptElement(subscript);
	moveTo(savedX, savedY);
	moveAcross(max(bboxWidth(supBBox), bboxWidth(subBBox)));
}

static void drawHat(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	double width = bboxWidth(bodyBBox);
	double savedX = currentX;
	double savedY = currentY;

	moveUp(bboxHeight(bodyBBox) + customAccentGap());
	GStartPath();
	GMoveTo(convertedX(), convertedY());
	moveUp(customHatHeight());
	moveAcross(width / 2);
	GLineTo(convertedX(), convertedY());
	moveUp(-customHatHeight());
	moveAcross(width / 2);
	GLineTo(convertedX(), convertedY());
	GEndPath();
	moveTo(savedX, savedY);
	drawElement(body);
}

static void drawBar(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	double savedX = currentX;
	double savedY = currentY;

	moveUp(bboxHeight(bodyBBox) + customAccentGap());
	GStartPath();
	GMoveTo(convertedX(), convertedY());
	moveAcross(bboxWidth(bodyBBox));
	GLineTo(convertedX(), convertedY());
	GEndPath();
	moveTo(savedX, savedY);
	drawElement(body);
}

static void drawAccent(SEXP expr)
{
	SEXP accent = CAR(expr);
	SEXP body = CADR(expr);
	double savedX = currentX;
	double savedY = currentY;

	if (hatAtom(accent))
		drawHat(body);
	else if (barAtom(accent))
		drawBar(body);
	else {
		moveAcross(accentHShift(body, accent));
		moveUp(accentVShift(body));
		drawAscii(accentAscii(accent));
		moveTo(savedX, savedY);
		drawElement(body);
	}
}

static void drawFraction(SEXP expr)
{
	SEXP numerator = CADR(expr);
	SEXP denominator = CADDR(expr);
	double savedX = currentX;
	double savedY = currentY;
	double fWidth = fractionWidth(numerator, denominator);
	double numHShift, denomHShift;

	numdenomHShift(numerator, denominator, &numHShift, &denomHShift);
	moveAcross(numHShift);
	moveUp(numeratorVShift(numerator));
#ifdef OLD
	drawScriptElement(numerator);
#else
	drawRatioElement(numerator);
#endif
	moveTo(savedX, savedY);
	moveUp(axisHeight());
	GStartPath();
	GMoveTo(convertedX(), convertedY());
	moveAcross(fWidth);
	GLineTo(convertedX(), convertedY());
	GEndPath();
	moveTo(savedX, savedY);
	moveAcross(denomHShift);
	moveUp(-denominatorVShift(denominator));
#ifdef OLD
	drawScriptElement(denominator);
#else
	drawRatioElement(denominator);
#endif
	moveTo(savedX + fWidth, savedY);
}

static void drawGroup(SEXP expr)
{
	drawAscii(groupOpenAscii());
	drawElement(CADR(expr));
	drawAscii(groupCloseAscii());
}

static void drawOperator(SEXP expr)
{
	drawAscii(operatorAscii(CAR(expr)));
	drawElement(CADR(expr));
}

		/* to be implemented */
static void drawCustomRadical(SEXP body)
{
	BBOX bodyBBox = elementBBox(body);
	double height = bboxHeight(bodyBBox);
	double depth = bboxDepth(bodyBBox);
	double width = bboxWidth(bodyBBox);
	double twiddleHeight = (height - depth) / 2;
	double savedX = currentX;
	double savedY = currentY;

	moveUp(0.8 * twiddleHeight);
	GStartPath();
	GMoveTo(convertedX(), convertedY());
	moveUp(0.2 * twiddleHeight);
	moveAcross(0.3 * customRadicalWidth());
	GLineTo(convertedX(), convertedY());
	moveUp(-(twiddleHeight + depth));
	moveAcross(0.3 * customRadicalWidth());
	GLineTo(convertedX(), convertedY());
	moveUp(depth + height + customRadicalGap());
	moveAcross(0.4 * customRadicalWidth());
	GLineTo(convertedX(), convertedY());
	moveAcross(customRadicalSpace() + width);
	GLineTo(convertedX(), convertedY());
	GEndPath();
	moveTo(savedX, savedY);
	moveAcross(customRadicalWidth() + customRadicalSpace());
	drawElement(body);
}

		/* to be implemented */
static void drawRadicalEx(SEXP body)
{
}

static void drawRadical(SEXP expr)
{
	SEXP body = CADR(expr);
	drawCustomRadical(body);
}

static void drawAbs(SEXP expr)
{
	SEXP body = CADR(expr);
	BBOX bodyBBox = elementBBox(expr);
	double height = bboxHeight(bodyBBox);
	double depth = bboxDepth(bodyBBox);
	double width = bboxWidth(bodyBBox);
	double savedX = currentX;
	double savedY = currentY;

	moveUp(-depth);
	GStartPath();
	GMoveTo(convertedX(), convertedY());
	moveUp(depth + height);
	GLineTo(convertedX(), convertedY());
	moveUp(-height);
	moveAcross(absSpace());
	drawElement(body);
	moveAcross(absSpace());
	moveUp(-depth);
	GMoveTo(convertedX(), convertedY());
	moveUp(depth + height);
	GLineTo(convertedX(), convertedY());
	GEndPath();
	moveUp(-height);
}

static void drawExpression(SEXP expr)
{
	int i;
	int numParams = length(expr) - 1;

	drawElement(CAR(expr));
	expr = CDR(expr);
	drawAscii(groupOpenAscii());
	for (i = 0; i < numParams; i++) {
		drawElement(CAR(expr));
		expr = CDR(expr);
		if (i < numParams - 1) {
			drawAscii(commaAscii());
			drawAscii(spaceAscii());
		}
	}
	drawAscii(groupCloseAscii());
}

static void drawFormula(SEXP);

static drawCurly(SEXP expr)
{
	drawFormula(CADR(expr));
}

static void drawFormula(SEXP expr)
{
	SEXP head = CAR(expr);

	if (binAtom(head))
		drawBin(expr);
	else if (superAtom(head))
		drawSuper(expr);
	else if (subAtom(head))
		drawSub(expr);
	else if (accentAtom(head))
		drawAccent(expr);
	else if (fractionAtom(head))
		drawFraction(expr);
	else if (groupAtom(head))
		drawGroup(expr);
	else if (operatorAtom(head))
		drawOperator(expr);
	else if (radicalAtom(head))
		drawRadical(expr);
	else if (absAtom(head))
		drawAbs(expr);
	else if (curlyAtom(head))
		drawCurly(expr);

	/* if expression is not a special mathematical notation */
	/* function then just reconstruct expression */

	else
		drawExpression(expr);
}

static void drawElement(SEXP expr)
{
	if (formulaExpression(expr))
		drawFormula(expr);
	else
		drawAtom(expr);
}

		/* functions forming the API */

static double half_pi = 1.57079632679489661922;

void GMathText(double x, double y, SEXP expr, double xc, double yc, double rot)
{
	BBOX expressionBBox;

	initFormulaSymbols();
	expressionBBox = elementBBox(expr);

	/* NOTE that x and y are in Figure coordinates */

	referenceX = xFigtoInch(x);
	referenceY = yFigtoInch(y);

	/* this will change to accomodate justification */

	currentX = referenceX - xc * bboxWidth(expressionBBox);
	currentY = referenceY - yc * bboxHeight(expressionBBox);
	currentAngle = rot;
	cosAngle = cos(rot / 90 * half_pi);
	sinAngle = sin(rot / 90 * half_pi);
	drawElement(expr);
}


#define XINVFMAP(x) ((x - GP->fig2dev.ax)/GP->fig2dev.bx)
#define YINVFMAP(y) ((y - GP->fig2dev.ay)/GP->fig2dev.by)

void GMMathText(SEXP str, int side, double line, int outer, double at, int las)
{
	double a, x, y, xadj, yadj;

	if (outer) {
		switch (side) {
		case 1:
			x = at;
			y = yChartoNDC(GP->cexbase * GP->mex * (GP->oma[0] - line + 1));
			x = DP->inner2dev.ax + DP->inner2dev.bx * x;
			y = DP->ndc2dev.ay + DP->ndc2dev.by * y;
			a = 0.0;
			xadj = GP->adj;
			yadj = 0.0;
			break;
		case 2:
			x = xChartoNDC(GP->cexbase * GP->mex * (GP->oma[1] - line));
			y = at;
			x = DP->ndc2dev.ax + DP->ndc2dev.bx * x;
			y = DP->inner2dev.ay + DP->inner2dev.by * y;
			a = 90.0;
			xadj = GP->adj;
			yadj = 0.0;
			break;
		case 3:
			x = at;
			y = 1.0 - yChartoNDC(GP->cexbase * GP->mex * (GP->oma[2] - line));
			x = DP->inner2dev.ax + DP->inner2dev.bx * x;
			y = DP->ndc2dev.ay + DP->ndc2dev.by * y;
			a = 0.0;
			xadj = GP->adj;
			yadj = 0.0;
			break;
		case 4:
			x = 1.0 - xChartoNDC(GP->cexbase * GP->mex * (GP->oma[3] - line));
			y = at;
			x = DP->ndc2dev.ax + DP->ndc2dev.bx * x;
			y = DP->inner2dev.ay + DP->inner2dev.by * y;
			a = 90.0;
			xadj = GP->adj;
			yadj = 0.0;
			break;
		}
		x = XINVFMAP(x);
		y = YINVFMAP(y);
		GMathText(x, y, str, xadj, yadj, a);
	}
	else {
		switch (side) {
		case 1:
			if (las == 2) {
				y = GP->plt[2] - yInchtoFig(yChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				x = XMAP(at) - xInchtoFig(xChartoInch(GP->cexbase * GP->mex * GP->yLineBias));
				a = 90.0;
				xadj = 1.0;
				yadj = 0.5;
			}
			else {
				y = GP->plt[2] - yInchtoFig(yChartoInch(GP->cexbase * GP->mex * (line + 1 - GP->yLineBias)));
				x = XMAP(at);
				a = 0.0;
				xadj = GP->adj;
				yadj = 0.0;
			}
			break;
		case 2:
			if (las == 1 || las == 2) {
				x = GP->plt[0] - xInchtoFig(xChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				y = YMAP(at) + yInchtoFig(yChartoInch(GP->cexbase * GP->mex * GP->yLineBias));
				a = 0.0;
				xadj = 1.0;
				yadj = 0.5;
			}
			else {
				x = GP->plt[0] - xInchtoFig(xChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				y = YMAP(at);
				a = 90.0;
				xadj = GP->adj;
				yadj = 0.0;
			}
			break;
		case 3:
			if (las == 2) {
				y = GP->plt[3] + yInchtoFig(yChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				x = XMAP(at) - xInchtoFig(xChartoInch(GP->cexbase * GP->mex * GP->yLineBias));
				a = 90.0;
				xadj = 0.0;
				yadj = 0.5;
			}
			else {
				y = GP->plt[3] + yInchtoFig(yChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				x = XMAP(at);
				a = 0.0;
				xadj = GP->adj;
				yadj = 0.0;
			}
			break;
		case 4:
			if (las == 1 || las == 2) {
				x = GP->plt[1] + xInchtoFig(xChartoInch(GP->cexbase * GP->mex * (line + GP->yLineBias)));
				y = YMAP(at) + yInchtoFig(yChartoInch(GP->cexbase * GP->mex * GP->yLineBias));
				a = 0.0;
				xadj = 0.0;
				yadj = 0.5;
			}
			else {
				x = GP->plt[1] + xInchtoFig(xChartoInch(GP->cexbase * GP->mex * (line + 1 - GP->yLineBias)));
				y = YMAP(at);
				a = 90.0;
				xadj = GP->adj;
				yadj = 0.0;
			}
			break;
		}
		GMathText(x, y, str, xadj, yadj, a);
	}
}
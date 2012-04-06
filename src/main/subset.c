/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997, 1998  The R Development Core Team
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  Vector and List Subsetting
 *
 *  There are three kinds of subscripting [, [[, and $.
 *  We have three different functions to compute these.
 *
 *
 *  Note on Matrix Subscripts
 *
 *  The special [ subscripting where dim(x) == ncol(subscript matrix)
 *  is handled inside vectorSubset. The subscript matrix is turned
 *  into a subscript vector of the appropriate size and then
 *  vectorSubset continues.  This provides coherence especially
 *  regarding attributes etc. (it would be quicker handle this case
 *  separately, but then we would have more to keep in step.
 */

#ifdef HAVE_CONFIG_H
#include <Rconfig.h>
#endif

#include "Defn.h"

/* ExtractSubset does the transfer of elements from "x" to "result" */
/* according to the integer subscripts given in "index". */

static SEXP ExtractSubset(SEXP x, SEXP result, SEXP index, SEXP call)
{
    int i, ii, n, nx, mode;
    SEXP tmp, tmp2;
    mode = TYPEOF(x);
    n = LENGTH(index);
    nx = length(x);
    tmp = result;

    if (x == R_NilValue)
	return x;

    for (i = 0; i < n; i++) {
	ii = INTEGER(index)[i];
	if (ii != NA_INTEGER)
	    ii--;
	switch (mode) {
	case LGLSXP:
	case INTSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER)
		INTEGER(result)[i] = INTEGER(x)[ii];
	    else
		INTEGER(result)[i] = NA_INTEGER;
	    break;
	case REALSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER)
		REAL(result)[i] = REAL(x)[ii];
	    else
		REAL(result)[i] = NA_REAL;
	    break;
	case CPLXSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER) {
		COMPLEX(result)[i] = COMPLEX(x)[ii];
	    }
	    else {
		COMPLEX(result)[i].r = NA_REAL;
		COMPLEX(result)[i].i = NA_REAL;
	    }
	    break;
	case STRSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER)
		STRING(result)[i] = STRING(x)[ii];
	    else
		STRING(result)[i] = NA_STRING;
	    break;
	case VECSXP:
	case EXPRSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER)
		VECTOR(result)[i] = VECTOR(x)[ii];
	    else
		VECTOR(result)[i] = R_NilValue;
	    break;
	case LISTSXP:
	case LANGSXP:
	    if (0 <= ii && ii < nx && ii != NA_INTEGER) {
		tmp2 = nthcdr(x, ii);
		CAR(tmp) = CAR(tmp2);
		TAG(tmp) = TAG(tmp2);
	    }
	    else
		CAR(tmp) = R_NilValue;
	    tmp = CDR(tmp);
	    break;
	default:
	    errorcall(call, "non-subsetable object\n");
	}
    }
    return result;
}


static SEXP VectorSubset(SEXP x, SEXP s, SEXP call)
{
    int n, mode, stretch = 1;
    SEXP index, result, attrib, nattrib;

    if (s == R_MissingArg)
	return duplicate(x);

    PROTECT(s);
    attrib = getAttrib(x, R_DimSymbol);

    /* Check to see if we have special matrix subscripting. */
    /* If we do, make a real subscript vector and protect it. */

    if (isMatrix(s) && isArray(x) && (isInteger(s) || isReal(s)) &&
	    ncols(s) == length(attrib)) {
	s = mat2indsub(attrib, s);
	UNPROTECT(1);
	PROTECT(s);
    }

    /* Convert to a vector of integer subscripts */
    /* in the range 1:length(x). */

    PROTECT(index = makeSubscript(x, s, &stretch));
    n = LENGTH(index);

    /* Allocate the result. */

    mode = TYPEOF(x);
    result = allocVector(mode, n);
    NAMED(result) = NAMED(x);

    PROTECT(result = ExtractSubset(x, result, index, call));
    if (result != R_NilValue &&
	(((attrib = getAttrib(x, R_NamesSymbol)) != R_NilValue) ||
	 ((attrib = getAttrib(x, R_DimNamesSymbol)) != R_NilValue &&
	  (attrib = GetRowNames(attrib)) != R_NilValue))) {
	nattrib = allocVector(TYPEOF(attrib), n);
	PROTECT(nattrib);
	nattrib = ExtractSubset(attrib, nattrib, index, call);
	setAttrib(result, R_NamesSymbol, nattrib);
	UNPROTECT(1);
    }
    UNPROTECT(3);
    return result;
}


SEXP MatrixSubset(SEXP x, SEXP s, SEXP call, int drop)
{
    SEXP attr, result, sr, sc;
    int nr, nc, nrs, ncs;
    int i, j, ii, jj, ij, iijj;

    nr = nrows(x);
    nc = ncols(x);

    /* Note that "s" is protected on entry. */
    /* The following ensures that pointers remain protected. */

    sr = CAR(s) = arraySubscript(0, CAR(s), x);
    sc = CADR(s) = arraySubscript(1, CADR(s), x);
    nrs = LENGTH(sr);
    ncs = LENGTH(sc);
    PROTECT(sr);
    PROTECT(sc);
    result = allocVector(TYPEOF(x), nrs*ncs);
    PROTECT(result);
    for (i = 0; i < nrs; i++) {
	ii = INTEGER(sr)[i];
	if (ii != NA_INTEGER) {
	    if (ii < 1 || ii > nr)
		errorcall(call, "subscript out of bounds\n");
	    ii--;
	}
	for (j = 0; j < ncs; j++) {
	    jj = INTEGER(sc)[j];
	    if (jj != NA_INTEGER) {
		if (jj < 1 || jj > nc)
		    errorcall(call, "subscript out of boundsn");
		jj--;
	    }
	    ij = i + j * nrs;
	    if (ii == NA_INTEGER || jj == NA_INTEGER) {
		switch (TYPEOF(x)) {
		case LGLSXP:
		case INTSXP:
		    INTEGER(result)[ij] = NA_INTEGER;
		    break;
		case REALSXP:
		    REAL(result)[ij] = NA_REAL;
		    break;
		case CPLXSXP:
		    COMPLEX(result)[ij].r = NA_REAL;
		    COMPLEX(result)[ij].i = NA_REAL;
		    break;
		case STRSXP:
		    STRING(result)[i] = NA_STRING;
		    break;
		}
	    }
	    else {
		iijj = ii + jj * nr;
		switch (TYPEOF(x)) {
		case LGLSXP:
		case INTSXP:
		    INTEGER(result)[ij] = INTEGER(x)[iijj];
		    break;
		case REALSXP:
		    REAL(result)[ij] = REAL(x)[iijj];
		    break;
		case CPLXSXP:
		    COMPLEX(result)[ij] = COMPLEX(x)[iijj];
		    break;
		case STRSXP:
		    STRING(result)[ij] = STRING(x)[iijj];
		    break;
		}
	    }
	}
    }
    if(nrs >= 0 && ncs >= 0) {
	PROTECT(attr = allocVector(INTSXP, 2));
	INTEGER(attr)[0] = nrs;
	INTEGER(attr)[1] = ncs;
	setAttrib(result, R_DimSymbol, attr);
	UNPROTECT(1);
    }

    /* The matrix elements have been transferred.  Now we need to */
    /* transfer the attributes.	 Most importantly, we need to subset */
    /* the dimnames of the returned value. */

    if (nrs >= 0 && ncs >= 0) {
	SEXP dimnames, newdimnames;
	dimnames = getAttrib(x, R_DimNamesSymbol);
	if (!isNull(dimnames)) {
	    PROTECT(newdimnames = allocVector(VECSXP, 2));
	    if (TYPEOF(dimnames) == VECSXP) {
		VECTOR(newdimnames)[0]
		    = ExtractSubset(VECTOR(dimnames)[0],
				    allocVector(STRSXP, nrs), sr, call);
		VECTOR(newdimnames)[1]
		    = ExtractSubset(VECTOR(dimnames)[1],
				    allocVector(STRSXP, ncs), sc, call);
	    }
	    else {
		VECTOR(newdimnames)[0]
		    = ExtractSubset(CAR(dimnames),
				    allocVector(STRSXP, nrs), sr, call);
		VECTOR(newdimnames)[1]
		    = ExtractSubset(CADR(dimnames),
				    allocVector(STRSXP, ncs), sc, call);
	    }
	    setAttrib(result, R_DimNamesSymbol, newdimnames);
	    UNPROTECT(1);
	}
    }
    /*  Probably should not do this:
    copyMostAttrib(x, result); */
    if (drop)
	DropDims(result);
    UNPROTECT(3);
    return result;
}


static SEXP ArraySubset(SEXP x, SEXP s, SEXP call, int drop)
{
    int i, j, k, ii, jj, mode, n;
    int **subs, *index, *offset, *bound;
    SEXP dimnames, p, q, r, result, xdims;
    char *vmaxsave;

    mode = TYPEOF(x);
    xdims = getAttrib(x, R_DimSymbol);
    k = length(xdims);

    vmaxsave = vmaxget();
    subs = (int**)R_alloc(k, sizeof(int*));
    index = (int*)R_alloc(k, sizeof(int));
    offset = (int*)R_alloc(k, sizeof(int));
    bound = (int*)R_alloc(k, sizeof(int));

    /* Construct a vector to contain the returned values. */
    /* Store its extents. */

    n = 1;
    r = s;
    for (i = 0; i < k; i++) {
	CAR(r) = arraySubscript(i, CAR(r), x);
	bound[i] = LENGTH(CAR(r));
	n *= bound[i];
	r = CDR(r);
    }
    PROTECT(result = allocVector(mode, n));
    r = s;
    for (i = 0; i < k; i++) {
	index[i] = 0;
	subs[i] = INTEGER(CAR(r));
	r = CDR(r);
    }
    offset[0] = 1;
    for (i = 1; i < k; i++)
	offset[i] = offset[i - 1] * INTEGER(xdims)[i - 1];

    /* Transfer the subset elements from "x" to "a". */

    for (i = 0; i < n; i++) {
	ii = 0;
	for (j = 0; j < k; j++) {
	    jj = subs[j][index[j]];
	    if (jj == NA_INTEGER) {
		ii = NA_INTEGER;
		goto assignLoop;
	    }
	    if (jj < 1 || jj > INTEGER(xdims)[j])
		errorcall(call, "subscript out of bounds\n");
	    ii += (jj - 1) * offset[j];
	}

      assignLoop:
	switch (mode) {
	case LGLSXP:
	    if (ii != NA_LOGICAL)
		LOGICAL(result)[i] = LOGICAL(x)[ii];
	    else
		LOGICAL(result)[i] = NA_LOGICAL;
	    break;
	case INTSXP:
	    if (ii != NA_INTEGER)
		INTEGER(result)[i] = INTEGER(x)[ii];
	    else
		INTEGER(result)[i] = NA_INTEGER;
	    break;
	case REALSXP:
	    if (ii != NA_INTEGER)
		REAL(result)[i] = REAL(x)[ii];
	    else
		REAL(result)[i] = NA_REAL;
	    break;
	case CPLXSXP:
	    if (ii != NA_INTEGER) {
		COMPLEX(result)[i] = COMPLEX(x)[ii];
	    }
	    else {
		COMPLEX(result)[i].r = NA_REAL;
		COMPLEX(result)[i].i = NA_REAL;
	    }
	    break;
	case STRSXP:
	    if (ii != NA_INTEGER)
		STRING(result)[i] = STRING(x)[ii];
	    else
		STRING(result)[i] = NA_STRING;
	    break;
	}
	if (n > 1) {
	    j = 0;
	    while (++index[j] >= bound[j]) {
		index[j] = 0;
		j = ++j % k;
	    }
	}
    }

    PROTECT(xdims = allocVector(INTSXP, k));
    for(i=0 ; i<k ; i++)
	INTEGER(xdims)[i] = bound[i];
    setAttrib(result, R_DimSymbol, xdims);
    UNPROTECT(1);

    /* The array elements have been transferred. */
    /* Now we need to transfer the attributes. */
    /* Most importantly, we need to subset the */
    /* dimnames of the returned value. */

    dimnames = getAttrib(x, R_DimNamesSymbol);
    if (dimnames != R_NilValue) {
	SEXP xdims;
	int j = 0;
	PROTECT(xdims = allocVector(VECSXP, k));
	if (TYPEOF(dimnames) == VECSXP) {
	    r = s;
	    for (i = 0; i < k ; i++) {
		if (bound[i] > 0) {
		    VECTOR(xdims)[j++]
			= ExtractSubset(VECTOR(dimnames)[i],
					allocVector(STRSXP, bound[i]),
					CAR(r), call);
		}
		r = CDR(r);
	    }
	}
	else {
	    p = dimnames;
	    q = xdims;
	    r = s;
	    for(i = 0 ; i < k; i++) {
		CAR(q) = allocVector(STRSXP, bound[i]);
		CAR(q) = ExtractSubset(CAR(p), CAR(q), CAR(r), call);
		p = CDR(p);
		q = CDR(q);
		r = CDR(r);
	    }
	}
	setAttrib(result, R_DimNamesSymbol, xdims);
	UNPROTECT(1);
    }
    copyMostAttrib(x, result);
    /* Free temporary memory */
    vmaxset(vmaxsave);
    if (drop)
	DropDims(result);
    UNPROTECT(1);
    return result;
}



static SEXP ExtractDropArg(SEXP el, int *drop)
{
    if(el == R_NilValue)
	return R_NilValue;
    if(TAG(el) == R_DropSymbol) {
	*drop = asLogical(CAR(el));
	if (*drop == NA_LOGICAL) *drop = 1;
	return ExtractDropArg(CDR(el), drop);
    }
    CDR(el) = ExtractDropArg(CDR(el), drop);
    return el;
}



/* The "[" subset operator.
 * This provides the most general form of subsetting. */

SEXP do_subset(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ans, dim, ax, px, x, subs;
    int drop, i, ndim, nsubs, type;

    /* If the first argument is an object and there is an */
    /* approriate method, we dispatch to that method, */
    /* otherwise we evaluate the arguments and fall through */
    /* to the generic code below.  Note that evaluation */
    /* retains any missing argument indicators. */

    if(DispatchOrEval(call, op, args, rho, &ans, 0))
	return(ans);

    /* Method dispatch has failed, we now */
    /* run the generic internal code. */

    /* By default we drop extents of length 1 */

    drop = 1;
    PROTECT(args = ans);
    ExtractDropArg(args, &drop);
    x = CAR(args);

    /* This was intended for compatibility with S, */
    /* but in fact S does not do this. */
    /* FIXME: replace the test by isNull ... ? */

    if (x == R_NilValue) {
	UNPROTECT(1);
	return x;
    }
    subs = CDR(args);
    if(0 == (nsubs = length(subs)))
	errorcall(call, "no index specified\n");
    type = TYPEOF(x);
    PROTECT(dim = getAttrib(x, R_DimSymbol));
    ndim = length(dim);

    /* Here coerce pair-based objects into generic vectors. */
    /* All subsetting takes place on the generic vector form. */

    if (isPairList(x)) {
	if (ndim > 1) {
	    PROTECT(ax = allocArray(VECSXP, dim));
	    setAttrib(ax, R_DimNamesSymbol, getAttrib(x, R_DimNamesSymbol));
	    setAttrib(ax, R_NamesSymbol, getAttrib(x, R_DimNamesSymbol));
	}
	else {
	    PROTECT(ax = allocVector(VECSXP, length(x)));
	    setAttrib(ax, R_NamesSymbol, getAttrib(x, R_NamesSymbol));
	}
	for(px = x, i = 0 ; px != R_NilValue ; px = CDR(px))
	    VECTOR(ax)[i++] = CAR(px);
    }
    else PROTECT(ax = x);

    if(!isVector(ax))
	errorcall(call, "object is not subsetable\n");

    /* This is the actual subsetting code. */
    /* The separation of arrays and matrices is purely an optimization. */

    if(nsubs == 1)
	ans = VectorSubset(ax, CAR(subs), call);
    else {
	if (nsubs != length(dim))
	    errorcall(call, "incorrect number of dimensions\n");
	if (nsubs == 2)
	    ans = MatrixSubset(ax, subs, call, drop);
	else
	    ans = ArraySubset(ax, subs, call, drop);
    }
    PROTECT(ans);

    /* Note: we do not coerce back to pair-based lists. */
    /* They are "defunct" in this version of R. */

    if (type == LANGSXP) {
	ax = ans;
	PROTECT(ans = allocList(LENGTH(ax)));
	if ( LENGTH(ax) > 0 )
	    TYPEOF(ans) = LANGSXP;
	for(px = ans, i = 0 ; px != R_NilValue ; px = CDR(px))
	    CAR(px) = VECTOR(ax)[i++];
	setAttrib(ans, R_DimSymbol, getAttrib(ax, R_DimSymbol));
	setAttrib(ans, R_DimNamesSymbol, getAttrib(ax, R_DimNamesSymbol));
	setAttrib(ans, R_NamesSymbol, getAttrib(ax, R_NamesSymbol));
    }
    else {
	PROTECT(ans);
    }
    setAttrib(ans, R_TspSymbol, R_NilValue);
    setAttrib(ans, R_ClassSymbol, R_NilValue);
    UNPROTECT(5);
    return ans;
}


/* The [[ subset operator.  It needs to be fast. */
/* The arguments to this call are evaluated on entry. */

SEXP do_subset2(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ans, dims, dimnames, index, subs, x;
    int i, ndims, nsubs, offset = 0;
    int drop = 1;

    /* If the first argument is an object and there is */
    /* an approriate method, we dispatch to that method, */
    /* otherwise we evaluate the arguments and fall */
    /* through to the generic code below.  Note that */
    /* evaluation retains any missing argument indicators. */

    if(DispatchOrEval(call, op, args, rho, &ans, 0))
	return(ans);

    /* Method dispatch has failed. */
    /* We now run the generic internal code. */

    PROTECT(args = ans);
    ExtractDropArg(args, &drop);
    x = CAR(args);

    /* This code was intended for compatibility with S, */
    /* but in fact S does not do this.	Will anyone notice? */

    if (x == R_NilValue) {
	UNPROTECT(1);
	return x;
    }

    /* Get the subscripting and dimensioning information */
    /* and check that any array subscripting is compatible. */

    subs = CDR(args);
    if(0 == (nsubs = length(subs)))
	errorcall(call, "no index specified\n");
    dims = getAttrib(x, R_DimSymbol);
    ndims = length(dims);
    if(nsubs > 1 && nsubs != ndims)
	errorcall(call, "incorrect number of subscripts\n");

    if (isVector(x) || isList(x) || isLanguage(x)) {

	if(nsubs == 1) {
	    offset = get1index(CAR(subs), getAttrib(x, R_NamesSymbol),
			       length(x), 1);
	    if (offset < 0 || offset >= length(x)) {
		/* a bold attempt to get the same */
		/* behaviour for $ and [[ */
		if (offset < 0 && (isNewList(x) ||
				   isExpression(x) ||
				   isList(x) ||
				   isLanguage(x))) {
		    UNPROTECT(1);
		    return R_NilValue;
		}
		else errorcall(call, "subscript out of bounds\n");
	    }
	}
	else {
	    /* Here we use the fact that: */
	    /* CAR(R_NilValue) = R_NilValue */
	    /* CDR(R_NilValue) = R_NilValue */
	    PROTECT(index = allocVector(INTSXP, nsubs));
	    dimnames = getAttrib(x, R_DimNamesSymbol);
	    for (i = 0; i < nsubs; i++) {
		INTEGER(index)[i] = get1index(CAR(subs),
					      VECTOR(dimnames)[i],
					      INTEGER(index)[i], 1);
		subs = CDR(subs);
		if (INTEGER(index)[i] < 0 ||
		    INTEGER(index)[i] >= INTEGER(dims)[i])
		    errorcall(call, "subscript out of bounds\n");
	    }
	    offset = 0;
	    for (i = (nsubs - 1); i > 0; i--)
		offset = (offset + INTEGER(index)[i]) * INTEGER(dims)[i - 1];
	    offset += INTEGER(index)[0];
	    UNPROTECT(1);
	}
    }
    else errorcall(call, "object is not subsettable\n");

    if(isPairList(x)) {
	ans = CAR(nthcdr(x, offset));
	NAMED(ans) = NAMED(x);
    }
    else if(isVectorList(x)) {
	ans = duplicate(VECTOR(x)[offset]);
    }
    else {
	ans = allocVector(TYPEOF(x), 1);
	switch (TYPEOF(x)) {
	case LGLSXP:
	case INTSXP:
	    INTEGER(ans)[0] = INTEGER(x)[offset];
	    break;
	case REALSXP:
	    REAL(ans)[0] = REAL(x)[offset];
	    break;
	case CPLXSXP:
	    COMPLEX(ans)[0] = COMPLEX(x)[offset];
	    break;
	case STRSXP:
	    STRING(ans)[0] = STRING(x)[offset];
	    break;
	default:
	    UNIMPLEMENTED("do_subset2");
	}
    }
    UNPROTECT(1);
    return ans;
}


/* A helper to partially match tags against a candidate. */
/* Returns: */
#define NO_MATCH      -1/* for no match */
#define EXACT_MATCH    0/* for a perfect match */
#define PARTIAL_MATCH  1/* for a partial match */

static int pstrmatch(SEXP target, SEXP input, int slen)
{
    char *st="";
    int k;

    if(target == R_NilValue) return -1;

    switch (TYPEOF(target)) {
    case SYMSXP:
	st = CHAR(PRINTNAME(target));
	break;
    case CHARSXP:
	st = CHAR(target);
	break;
    }
    k = strncmp(st, CHAR(input), slen);
    if(k == 0) {
	if (strlen(st) == slen)
	    return EXACT_MATCH;
	else
	    return PARTIAL_MATCH;
    }
    else return NO_MATCH;
}


/* The $ subset operator.  We need to be sure to only evaluate */
/* the first argument.	The second will be a symbol that needs */
/* to be matched, not evaluated. */

SEXP do_subset3(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP x, y, input, nlist;
    int slen;

    checkArity(op, args);

    PROTECT(x = eval(CAR(args), env));
    nlist = CADR(args);
    if(isSymbol(nlist) )
	input = PRINTNAME(nlist);
    else if(isString(nlist) )
	input = STRING(nlist)[0];
    else {
	errorcall(call, "invalid subscript type\n");
	return R_NilValue; /*-Wall*/
    }

    /* Optimisation to prevent repeated recalculation */
    slen = strlen(CHAR(input));

    /* If this is not a list object we return NULL. */
    /* Or should this be allocVector(VECSXP, 0)? */

    if (isPairList(x)) {
	SEXP xmatch=R_NilValue;
	int havematch;
	UNPROTECT(1);
	havematch = 0;
	for (y = x ; y != R_NilValue ; y = CDR(y)) {
	    switch(pstrmatch(TAG(y), input, slen)) {
	    case EXACT_MATCH:
		y = CAR(y);
		NAMED(y) = NAMED(x);
		return y;
		break;
	    case PARTIAL_MATCH:
		havematch++;
		xmatch = y;
		break;
	    }
	}
	if (havematch == 1) {
	    y = CAR(xmatch);
	    NAMED(y) = NAMED(x);
	    return y;
	}
	return R_NilValue;
    }
    else if (isVectorList(x)) {
	int i, n, havematch, imatch=-1;
	nlist = getAttrib(x, R_NamesSymbol);
	UNPROTECT(1);
	n = length(nlist);
	havematch = 0;
	for (i = 0 ; i < n ; i = i + 1) {
	    switch(pstrmatch(STRING(nlist)[i], input, slen)) {
	    case EXACT_MATCH:
		y = VECTOR(x)[i];
		NAMED(y) = NAMED(x);
		return y;
		break;
	    case PARTIAL_MATCH:
		havematch++;
		imatch = i;
		break;
	    }
	}
	if(havematch ==1) {
	    y = VECTOR(x)[imatch];
	    NAMED(y) = NAMED(x);
	    return y;
	}
	return R_NilValue;
    }
    UNPROTECT(1);
    return R_NilValue;
}
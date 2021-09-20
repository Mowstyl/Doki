#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "funmatrix.h"

/* Constructor */
FunctionalMatrix*
new_FunctionalMatrix(NATURAL_TYPE n_rows, NATURAL_TYPE n_columns,
                     COMPLEX_TYPE (*fun)(NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, NATURAL_TYPE, void*),
                     void *argv)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = n_rows;
        pFM->c = n_columns;
        pFM->f = fun;
        pFM->s = COMPLEX_ONE;
        pFM->transpose = 0;
        pFM->conjugate = 0;
        pFM->simple = 1;
        pFM->argv = argv;
    }

    return pFM;
}

/* Get the element (i, j) from the matrix a */
int
getitem(FunctionalMatrix *a, NATURAL_TYPE i, NATURAL_TYPE j, COMPLEX_TYPE *sol)
{
    unsigned int k;
    NATURAL_TYPE aux;
    int result = 1;
    COMPLEX_TYPE aux1 = COMPLEX_ZERO,
                 aux2 = COMPLEX_ZERO;

    *sol = complex_init(NAN, NAN);
    if (i < a->r && j < a->c) {
        if (a->transpose) {
            aux = i;
            i = j;
            j = aux;
        }

        if (a->simple) {
            *sol = a->f(i, j, a->r, a->c, a->argv);
        }
        else {
            switch (a->op) {
                case 0: /* Matrix addition */
                    if (getitem(a->A, i, j, &aux1) && getitem(a->B, i, j, &aux2)) {
                        *sol = complex_sum(aux1, aux2);
                    }
                    else {
                        printf("Error while operating!\n");
                        result = 0;
                    }
                    break;
                case 1: /* Matrix subtraction */
                    if (getitem(a->A, i, j, &aux1) && getitem(a->B, i, j, &aux2)) {
                        *sol = complex_sub(aux1, aux2);
                    }
                    else {
                        printf("Error while operating!\n");
                        result = 0;
                    }
                    break;
                case 2: /* Matrix multiplication    */
                    *sol = COMPLEX_ZERO;
                    for (k = 0; k < a->A->c; k++) {
                        if (getitem(a->A, i, k, &aux1) && getitem(a->B, k, j, &aux2)) {
                            *sol = complex_sum(*sol, complex_mult(aux1, aux2));
                        }
                        else {
                            printf("Error while operating!\n");
                            result = 0;
                            break;
                        }
                    }
                    break;
                case 3: /* Entity-wise multiplication */
                    if (getitem(a->A, i, j, &aux1) && getitem(a->B, i, j, &aux2)) {
                        *sol = complex_mult(aux1, aux2);
                    }
                    else {
                        printf("Error while operating!\n");
                        result = 0;
                    }
                    break;

                case 4: /* Kronecker product */
                    if (getitem(a->A, i/a->B->r, j/a->B->c, &aux1) && getitem(a->B, i%a->B->r, j%a->B->c, &aux2)) {
                        *sol = complex_mult(aux1, aux2);
                    }
                    else {
                        printf("Error while operating!\n");
                        result = 0;
                    }
                    break;

                default:
                    printf("Unknown option: %d\n", a->op);
                    result = 0;
            }
        }

        if (result && a->conjugate) {
            *sol = conj(*sol);
        }
    }
    else {
        printf("(" NATURAL_STRING_FORMAT ", " NATURAL_STRING_FORMAT ") is out of bounds!\n Matrix dimensions: (" NATURAL_STRING_FORMAT ", " NATURAL_STRING_FORMAT ")\n", i, j, a->r, a->c);
        result = 0;
    }

    if (result) {
        *sol = complex_mult(*sol, a->s);
    }

    return result;
}

/* Addition */
FunctionalMatrix*
madd(FunctionalMatrix *a, FunctionalMatrix *b)
{
    FunctionalMatrix* pFM = NULL;

    /* if the dimensions allign (nxm .* nxm)*/
    if (a->r == b->r && a->c == b->c) {
        pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));
        if (pFM != NULL) {
            pFM->r = a->r;
            pFM->c = a->c;
            pFM->s = COMPLEX_ONE;
            pFM->A = a;
            pFM->B = b;
            pFM->op = 0;
            pFM->simple = 0;
        }
    }

    return pFM;
}

/* Subtraction */
FunctionalMatrix*
msub(FunctionalMatrix *a, FunctionalMatrix *b)
{
    FunctionalMatrix* pFM = NULL;

    /* if the dimensions allign (nxm .* nxm)*/
    if (a->r == b->r && a->c == b->c) {
        pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));
        if (pFM != NULL) {
            pFM->r = a->r;
            pFM->c = a->c;
            pFM->s = COMPLEX_ONE;
            pFM->A = a;
            pFM->B = b;
            pFM->op = 0;
            pFM->simple = 0;
        }
    }

    return pFM;
}

/* Scalar product */
FunctionalMatrix*
mprod(COMPLEX_TYPE r, FunctionalMatrix *a)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = a->r;
        pFM->c = a->c;
        pFM->f = a->f;
        pFM->s = complex_mult(a->s, r);
        pFM->A = a->A;
        pFM->B = a->B;
        pFM->op = a->op;
        pFM->simple = a->simple;
        pFM->argv = a->argv;
    }

    return pFM;
}

/* Scalar division */
FunctionalMatrix*
mdiv(COMPLEX_TYPE r, FunctionalMatrix *a)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = a->r;
        pFM->c = a->c;
        pFM->f = a->f;
        pFM->s = complex_div(a->s, r);
        pFM->A = a->A;
        pFM->B = a->B;
        pFM->op = a->op;
        pFM->simple = a->simple;
        pFM->argv = a->argv;
    }

    return pFM;
}

/* Matrix multiplication */
FunctionalMatrix*
matmul(FunctionalMatrix *a, FunctionalMatrix *b)
{
    FunctionalMatrix* pFM = NULL;

    /* if the dimensions allign (uxv * vxw) */
    if (a->c == b->r) {
        pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));
        if (pFM != NULL) {
            pFM->r = a->r;
            pFM->c = b->c;
            pFM->s = COMPLEX_ONE;
            pFM->A = a;
            pFM->B = b;
            pFM->op = 2;
            pFM->simple = 0;
        }
    }

    return pFM;
}

/* Entity-wise multiplication */
FunctionalMatrix*
ewmul(FunctionalMatrix *a, FunctionalMatrix *b)
{
    FunctionalMatrix* pFM = NULL;

    if (a->r == b->r && a->c == b->c) { /* if the dimensions allign (nxm .* nxm)*/
        pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));
        if (pFM != NULL) {
            pFM->r = a->r;
            pFM->c = a->c;
            pFM->s = COMPLEX_ONE;
            pFM->A = a;
            pFM->B = b;
            pFM->op = 3;
            pFM->simple = 0;
        }
    }
    else if (a->r == 1 && b->c == 1) { /* row .* column */
        pFM = matmul(b, a);
    }
    else if (b->r == 1 && a->c == 1) {
        pFM = matmul(a, b);
    }

    return pFM;
}

/* Kronecker product */
FunctionalMatrix*
kron(FunctionalMatrix *a, FunctionalMatrix *b)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = a->r * b->r;
        pFM->c = a->c * b->c;
        pFM->s = COMPLEX_ONE;
        pFM->A = a;
        pFM->B = b;
        pFM->op = 4;
        pFM->simple = 0;
    }

    return pFM;
}

/* Transpose */
FunctionalMatrix*
transpose(FunctionalMatrix *m)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = m->r;
        pFM->c = m->c;
        pFM->f = m->f;
        pFM->s = m->s;
        pFM->A = m->A;
        pFM->B = m->B;
        pFM->op = m->op;
        pFM->transpose = !m->transpose;
        pFM->conjugate = m->conjugate;
        pFM->simple = 0;
    }

    return pFM;
}

/* Hermitian transpose */
FunctionalMatrix*
dagger(FunctionalMatrix *m)
{
    FunctionalMatrix* pFM = (FunctionalMatrix*) malloc(sizeof (FunctionalMatrix));

    if (pFM != NULL) {
        pFM->r = m->r;
        pFM->c = m->c;
        pFM->f = m->f;
        pFM->s = m->s;
        pFM->A = m->A;
        pFM->B = m->B;
        pFM->op = m->op;
        pFM->transpose = !m->transpose;
        pFM->conjugate = !m->conjugate;
        pFM->simple = 0;
    }

    return pFM;
}

NATURAL_TYPE
_GetElemIndex(int value, NATURAL_TYPE position, int      bit)
{
    NATURAL_TYPE index = 0,
                 aux = 1;

    if ((value == 0 || value == 1) && bit >= 0) {
        if (bit != 0) {
            aux = (NATURAL_TYPE) (2 << (bit - 1));
        }
        index = position % aux + (position / aux) * (aux << 1) + value * aux;
    }

    return index;
}

COMPLEX_TYPE
_PartialTFunct(NATURAL_TYPE i, NATURAL_TYPE j,
               #ifndef _MSC_VER
               NATURAL_TYPE unused1 __attribute__((unused)), NATURAL_TYPE unused2 __attribute__((unused)),
               #else
               NATURAL_TYPE unused1, NATURAL_TYPE unused2,
               #endif
               void *items)
{
    COMPLEX_TYPE sol = COMPLEX_ZERO,
                 aux = COMPLEX_ZERO;
    _MatrixElem *me;

    if (items != NULL) {
        me = (_MatrixElem*) items;

        if (getitem(me->m, _GetElemIndex(0, i, me->e), _GetElemIndex(0, j, me->e), &sol) &&
                getitem(me->m, _GetElemIndex(1, i, me->e), _GetElemIndex(1, j, me->e), &aux)) {
            sol = complex_sum(sol, aux);
        }
    }

    return sol;
}

/* Partial trace */
FunctionalMatrix*
partial_trace(FunctionalMatrix *m, int elem)
{
    FunctionalMatrix *pt = NULL;
    _MatrixElem *me = NULL;

    if (m != NULL && m->r == m->c && elem >= 0) {
        me = (_MatrixElem*) malloc(sizeof (_MatrixElem));
        if (me != NULL) {
            me->m = m;
            me->e = elem;
            pt = new_FunctionalMatrix (m->r >> 1, m->c >> 1, _PartialTFunct, me);
        }
    }

    return pt;
}


NATURAL_TYPE
rows(FunctionalMatrix *m)
{
    return m->r;
}

NATURAL_TYPE
columns(FunctionalMatrix *m)
{
    return m->c;
}

int
_bytes_added(int sprintfRe)
{
    return (sprintfRe > 0) ? sprintfRe : 0;
}

/* Gets the size in memory */
size_t
getMemory(FunctionalMatrix *m)
{
    size_t total;

    total = sizeof (*m);
    if (!m->simple) {
        total += getMemory(m->A);
        total += getMemory(m->B);
    }

    return total;
}

/* Print matrix */
char*
FM_toString(FunctionalMatrix *a)
{
    char *text;
    COMPLEX_TYPE it;
    NATURAL_TYPE i, j;
    int length = 0;
    const NATURAL_TYPE MAX_BUF = a->r * a->c * (2 * (DECIMAL_PLACES + 7) + 2) + 2;
    // numero de elementos (r * c) multiplicado por numero de cifras significativas establecidas para cada numero
    // por 2 (son complejos) mas 7 (1 del signo, otro del . y 5 del exponente e-001) mas 2, uno de la i y otro del
    // espacio/;/] que hay despues de cada numero. Al final se suman 2, uno para el corchete inicial y otro para \0.

    text = (char*) malloc(MAX_BUF);

    it = COMPLEX_ZERO;
    if (text != NULL) {
        length += _bytes_added(snprintf(text + length, MAX_BUF - length, "["));
        for (i = 0; i < a->r; i++) {
            for (j = 0; j < a->c; j++) {
                if (getitem(a, i, j, &it) && !isnan(creal(it)) && !isnan(cimag(it))) {
                    if (cimag(it) >= 0) {
                        length += _bytes_added(snprintf(text + length, MAX_BUF - length,
                                                        REAL_STRING_FORMAT "+" REAL_STRING_FORMAT "i",
                                                        creal(it), cimag(it)));
                    }
                    else {
                        length += _bytes_added(snprintf(text + length, MAX_BUF - length,
                                                        REAL_STRING_FORMAT "-" REAL_STRING_FORMAT "i",
                                                        creal(it), cimag(it)));
                    }
                }
                else {
                    length += _bytes_added(snprintf(text + length, MAX_BUF - length, "ERR"));
                }
                if (j < a->c - 1) {
                    length += _bytes_added(snprintf(text + length, MAX_BUF - length, " "));
                }
            }

            if (i < a->r - 1) {
                length += _bytes_added(snprintf(text + length, MAX_BUF - length, ";"));
            }
        }
        length += _bytes_added(snprintf(text + length, MAX_BUF - length, "]"));
        *(text+length) = '\0';
    }

    return text;
}

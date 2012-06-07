/*=============================================================================

    This file is part of FLINT.

    FLINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2012 Sebastian Pancratz
 
******************************************************************************/

#include "fmpz_mod_poly.h"
#include "ulong_extras.h"
#include "qadic.h"

/*
    Assumes that \code{len1} and \code{len2} are positive but at 
    most~$d$, and also that \code{len1} is at least $6$.

    The latter assumption guarantees that $\ceil{n/B} \geq 2$, 
    i.e.\ $n \geq 2B$ so $n \geq 2 \ceil{\sqrt{n}}$.

 */

static void 
_fmpz_mod_poly_compose_mod_paterson_stockmeyer(fmpz *rop, 
                           const fmpz *op1, long len1, 
                           const fmpz *op2, long len2, 
                           const fmpz *a, const long *j, long lena, 
                           const fmpz_t p)
{
    const long d = j[lena - 1];

    if (len2 == 1)
    {
        _fmpz_mod_poly_evaluate_fmpz(rop, op1, len1, op2, p);
        _fmpz_vec_zero(rop + 1, d - 1);
    }
    else
    {
        const long B = n_sqrt(len1);
        long i, k;
        fmpz *c, *m, *s, *t;

        /*
            {t + i*d, d} is op2^{i+1}, for 0 <= i < B.
            We have one extra block of d-1 coefficients in t 
            to allow for the computation of products.
         */
        t = _fmpz_vec_init(B * d + d - 1);

        _fmpz_vec_set(t, op2, len2);
        _fmpz_vec_zero(t + len2, d - len2);

        for (i = 1; i < B; i++)
        {
            _fmpz_mod_poly_mul(t + i*d, t + (i-1)*d, d, op2, len2, p);
            _fmpz_mod_poly_reduce(t + i*d, d + len2 - 1, a, j, lena, p);
        }

        c = _fmpz_vec_init(d);
        m = _fmpz_vec_init(2*d - 1);
        s = _fmpz_vec_init(2*d - 1);

        /* Compute the bottom coefficient directly in rop */
        k = 0;
        {
            _fmpz_vec_zero(rop, d);
            fmpz_set(rop + 0, op1 + 0);
            for (i = 1; i < B; i++)
            {
                _fmpz_mod_poly_scalar_mul_fmpz(s, t + (i-1) * d, d, op1 + (i + B*k), p);
                _fmpz_mod_poly_add(rop, rop, d, s, d, p);
            }
        }

        _fmpz_vec_set(m, t + (B-1)*d, d);

        for (k = 1; k < (len1 + (B - 1)) / B - 1; k++)
        {
            /* Compute the coefficient polynomial in c */
            _fmpz_vec_zero(c, d);
            fmpz_set(c + 0, op1 + (0 + B*k));
            for (i = 1; i < B; i++)
            {
                _fmpz_mod_poly_scalar_mul_fmpz(s, t + (i-1) * d, d, op1 + (i + B*k), p);
                _fmpz_mod_poly_add(c, c, d, s, d, p);
            }

            /* Compute the product of the coeffient and the monomial in s */
            _fmpz_mod_poly_mul(s, c, d, m, d, p);
            _fmpz_mod_poly_reduce(s, 2*d - 1, a, j, lena, p);

            /* Update the sum */
            _fmpz_mod_poly_add(rop, rop, d, s, d, p);

            /* Update the monomial to (t^B)^{j+1} */
            _fmpz_mod_poly_mul(s, m, d, t + (B-1)*d, d, p);
            _fmpz_mod_poly_reduce(s, 2*d - 1, a, j, lena, p);
            _fmpz_vec_set(m, s, d);
        }

        /* Last step, j = \ceil{len1/B} - 1.  Possibly fewer terms.. */
        {
            /* Compute the coefficient polynomial in c */
            _fmpz_vec_zero(c, d);
            fmpz_set(c + 0, op1 + (0 + B*k));
            for (i = 1; i + B*k < len1; i++)
            {
                _fmpz_mod_poly_scalar_mul_fmpz(s, t + (i-1) * d, d, op1 + (i + B*k), p);
                _fmpz_mod_poly_add(c, c, d, s, d, p);
            }

            /* Compute the product of the coeffient and the monomial in s */
            _fmpz_mod_poly_mul(s, c, d, m, d, p);
            _fmpz_mod_poly_reduce(s, 2*d - 1, a, j, lena, p);

            /* Update the sum */
            _fmpz_mod_poly_add(rop, rop, d, s, d, p);
        }

        _fmpz_vec_clear(t, B * d + d - 1);
        _fmpz_vec_clear(c, d);
        _fmpz_vec_clear(m, 2*d - 1);
        _fmpz_vec_clear(s, 2*d - 1);
    }
}

static void 
_fmpz_mod_poly_compose_mod_horner(fmpz *rop, 
                           const fmpz *op1, long len1, 
                           const fmpz *op2, long len2, 
                           const fmpz *a, const long *j, long lena, 
                           const fmpz_t p)
{
    const long d = j[lena - 1];

    if (len1 == 1)
    {
        fmpz_set(rop, op1);
        _fmpz_vec_zero(rop + 1, d - 1);
    }
    else if (len2 == 1)
    {
        _fmpz_mod_poly_evaluate_fmpz(rop, op1, len1, op2, p);
        _fmpz_vec_zero(rop + 1, d - 1);
    }
    else
    {
        long i;
        fmpz *t;

        t = _fmpz_vec_init(2*d - 1);

        i = len1 - 1;

        _fmpz_mod_poly_scalar_mul_fmpz(rop, op2, len2, op1 + i, p);
        _fmpz_vec_zero(rop + len2, d - len2);
        i--;
        if (i >= 0)
        {
            fmpz_add(rop, rop, op1 + i);
            if (fmpz_cmpabs(rop, p) >= 0)
                fmpz_sub(rop, rop, p);
        }

        while (i > 0)
        {
            i--;
            _fmpz_mod_poly_mul(t, rop, d, op2, len2, p);
            _fmpz_mod_poly_reduce(t, d + len2 - 1, a, j, lena, p);
            _fmpz_mod_poly_add(rop, t, d, op1 + i, 1, p);
        }

        _fmpz_vec_clear(t, 2*d - 1);
    }
}

/* 
    Computes the composition $f(g(X))$ modulo the sparse polynomial 
    given by the data \code{(a, j, lena)}, which is assumed to be 
    of degree~$d \geq 2$.

    Sets the vector \code{(rop, d)}.

    Assumes that \code{len1} and \code{len2} are positive but at 
    most~$d$.

    Does not support aliasing.
 */

static void 
_fmpz_mod_poly_compose_mod(fmpz *rop, 
                           const fmpz *op1, long len1, 
                           const fmpz *op2, long len2, 
                           const fmpz *a, const long *j, long lena, 
                           const fmpz_t p)
{
    if (len1 < 6)
    {
        _fmpz_mod_poly_compose_mod_horner(rop, op1, len1, op2, len2, a, j, lena, p);
    }
    else
    {
        _fmpz_mod_poly_compose_mod_paterson_stockmeyer(rop, op1, len1, op2, len2, a, j, lena, p);
    }
}

void _qadic_frobenius_a(fmpz *rop, long exp, 
                    const fmpz *a, const long *j, long lena, 
                    const fmpz_t p, long N)
{
    const long d = j[lena - 1];

    long *e, i, n;
    fmpz *pow, *f1, *f2, *inv, *s, *t;

    n = FLINT_CLOG2(N) + 1;

    e = flint_malloc(n * sizeof(long));
    for (e[i = 0] = N; e[i] > 1; i++)
        e[i + 1] = (e[i] + 1) / 2;

    pow = _fmpz_vec_init(n);
    f1  = _fmpz_vec_init(d + 1);
    f2  = _fmpz_vec_init(d);
    inv = _fmpz_vec_init(2*d - 1);
    s   = _fmpz_vec_init(2*d - 1);
    t   = _fmpz_vec_init(2*d - 1);

    /* Compute powers of p */
    {
        fmpz_one(t);
        fmpz_set(pow + i, p);
    }
    for (i--; i >= 1; i--)
    {
        if (e[i] & 1L)
        {
            fmpz_mul(pow + i, t, pow + (i + 1));
            fmpz_mul(t, t, t);
        }
        else
        {
            fmpz_mul(t, t, pow + (i + 1));
            fmpz_mul(pow + i, pow + (i + 1), pow + (i + 1));
        }
    }
    {
        if (e[i] & 1L)
            fmpz_mul(pow + i, t, pow + (i + 1));
        else
            fmpz_mul(pow + i, pow + (i + 1), pow + (i + 1));
    }

    /* Dense representation of f and f' */
    {
        long k;

        for (k = 0; k < lena; k++)
            fmpz_set(f1 + j[k], a + k);
        for (k = 1; k < lena; k++)
            fmpz_mul_ui(f2 + (j[k] - 1), a + k, j[k]);
    }

    /* Run Newton iteration */
    i = n - 1;
    {
        fmpz op[2] = {0L, 1L};

        fmpz_pow_ui(t, p, exp);
        _qadic_pow(rop, op, 2, t, a, j, lena, pow + i);
        _fmpz_mod_poly_compose_mod(t, f2, d, rop, d, a, j, lena, pow + i);
        _qadic_inv(inv, t, d, a, j, lena, p, 1);
    }
    for (i--; i >= 0; i--)
    {
        _fmpz_mod_poly_compose_mod(s, f1, d + 1, rop, d, a, j, lena, pow + i);
        _fmpz_mod_poly_mul(t, s, d, inv, d, pow + i);
        _fmpz_mod_poly_reduce(t, 2*d - 1, a, j, lena, pow + i);
        _fmpz_mod_poly_sub(rop, rop, d, t, d, pow + i);

        if (i > 0)
        {
            _fmpz_mod_poly_compose_mod(s, f2, d, rop, d, a, j, lena, pow + i);
            _fmpz_mod_poly_mul(t, inv, d, s, d, pow + i);
            _fmpz_mod_poly_reduce(t, 2*d - 1, a, j, lena, pow + i);
            fmpz_sub_ui(t, t, 2);
            if (fmpz_sgn(t) < 0)
                fmpz_add(t, t, pow + i);
            _fmpz_mod_poly_neg(t, t, d, pow + i);
            _fmpz_mod_poly_mul(s, inv, d, t, d, pow + i);
            _fmpz_mod_poly_reduce(s, 2*d - 1, a, j, lena, pow + i);

            /* SWAP(inv, s).  Requires the arrays to be of the same size. */
            {
                fmpz *__t;

                __t = inv;
                inv = s;
                s   = __t;
            }
        }
    }

    _fmpz_vec_clear(pow, n);
    _fmpz_vec_clear(f1, d + 1);
    _fmpz_vec_clear(f2, d);
    _fmpz_vec_clear(inv, 2*d - 1);
    _fmpz_vec_clear(s, 2*d - 1);
    _fmpz_vec_clear(t, 2*d - 1);
    flint_free(e);
}

void _qadic_frobenius(fmpz *rop, const fmpz *op, long len, long e, 
                  const fmpz *a, const long *j, long lena, 
                  const fmpz_t p, long N)
{
    const long d = j[lena - 1];

    if (len == 1)  /* op is in Zp, not just Zq */
    {
        _fmpz_vec_set(rop, op, len);
        _fmpz_vec_zero(rop + len, d - len);
    }
    else if (N == 1)
    {
        fmpz_t t;

        fmpz_init(t);
        fmpz_pow_ui(t, p, e);
        _qadic_pow(rop, op, len, t, a, j, lena, p);
        fmpz_clear(t);
    }
    else
    {
        fmpz *t;
        fmpz_t pow;

        t = _fmpz_vec_init(2*d - 1);
        fmpz_init(pow);
        fmpz_pow_ui(pow, p, N);

        _qadic_frobenius_a(t, e, a, j, lena, p, N);

        _fmpz_mod_poly_compose_mod(rop, op, len, t, d, a, j, lena, pow);
        _fmpz_vec_zero(rop + d, d - 1);

        _fmpz_vec_clear(t, 2*d - 1);
        fmpz_clear(pow);
    }
}

void qadic_frobenius(qadic_t rop, const qadic_t op, long e, const qadic_ctx_t ctx)
{
    const long N = (&ctx->pctx)->N;
    const long d = qadic_ctx_degree(ctx);

    e = e % d;
    if (e < 0)
        e += d;

    if (qadic_is_zero(op) || op->val >= N)
    {
        qadic_zero(rop);
    }
    else if (e == 0)
    {
        padic_poly_set(rop, op);
        padic_poly_reduce(rop, &ctx->pctx);
    }
    else
    {
        fmpz *t;

        if (rop == op)
        {
            t = _fmpz_vec_init(2 * d - 1);
        }
        else
        {
            padic_poly_fit_length(rop, 2 * d - 1);
            t = rop->coeffs;
        }

        _qadic_frobenius(t, op->coeffs, op->length, e, 
                     ctx->a, ctx->j, ctx->len, (&ctx->pctx)->p, N - op->val);

        if (rop == op)
        {
            _fmpz_vec_clear(rop->coeffs, rop->alloc);
            rop->coeffs = t;
            rop->alloc  = 2 * d - 1;
            rop->length = d;
        }
        else
        {
            rop->val = op->val;
            _padic_poly_set_length(rop, d);
        }
        _padic_poly_normalise(rop);
    }
}

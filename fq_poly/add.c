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

    Copyright (C) 2012 Andres Goens

******************************************************************************/

#include "fq_poly.h"

void
fq_poly_add(fq_poly_t res, const fq_poly_t poly1,
            const fq_poly_t poly2)
{
    long len,i;
    fq_t s;

    fq_init(s);

    if(!fq_ctx_equal(poly1->ctx,poly2->ctx))
    {
        printf("Exception (fq_poly_add) polynomials with different fq_ctx.\n");
        abort();
    }

    len = (poly1->length < poly2->length ? poly2->length : poly1->length); /* max(poly1->len,poly2->len) */
    _fq_poly_set_length(res,len);

    for(i=0;i<len;i++)
    {
        if(i>poly1->length) fq_set(res->coeffs +i,poly2->coeffs + i); /*defn of len assures then that poly2->coefs + i points to something */
        if(i>poly2->length) fq_set(res->coeffs +i,poly1->coeffs + i); 
        else
        {
            fq_add(s,poly1->coeffs +i, poly2->coeffs + i,poly1->ctx);
            fq_set(res->coeffs+i, s);
        }
    }
}
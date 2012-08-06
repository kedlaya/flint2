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
fq_poly_change_ctx(fq_poly_t poly, const fq_ctx_t ctx)
{
    if(!fq_ctx_equal(poly->ctx,ctx))
    {
        fq_ctx_clear(poly->ctx);
        fq_ctx_init_conway(poly->ctx,ctx->pctx.p,fq_ctx_dim(ctx),ctx->var,ctx->pctx.mode);
    }
}
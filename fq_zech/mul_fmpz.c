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

    Copyright (C) 2013 Mike Hansen

******************************************************************************/

#include "fq_zech.h"

void
fq_zech_mul_fmpz(fq_zech_t rop, const fq_zech_t op, const fmpz_t x,
                 const fq_zech_ctx_t ctx)
{
    mp_limb_t ux;
    fmpz_t y;

    fmpz_init(y);
    fmpz_mod_ui(y, x, ctx->p);

    ux = fmpz_get_ui(y);

    fmpz_clear(y);

    fq_zech_mul_ui(rop, op, ux, ctx);
}

/* Rectangular class for Midnight Commander widgets

   Copyright (C) 2011
   The Free Software Foundation, Inc.

   Written by: 2011 Andrew Borodin

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MC_WIDGET_RECT_H
#define MC_WIDGET_RECT_H

#include <glib.h>

#include <string.h>     /* memcmp() */

G_BEGIN_DECLS

/*** typedefs (not structures) and defined constants *********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures) ****************************************/

typedef _mc_rect_t mc_rect_t;

struct _mc_rect_t
{
    gssize x;
    gssize y;
    gsize w;
    gsize h;
};

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

static inline void
mc_rect_init (mc_rect_t * r, gssize x, gssize y, gsize w, gsize h)
{
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
}

static inline mc_rect_t *
mc_rect_dup (const mc_rect_t * r)
{
    return g_memdup (r, sizeof (mc_rect_t));
}

static inline void
mc_rect_move (mc_rect_t * r, gssize dx, gssize dy)
{
    r->x += dx;
    r->y += dy;
}

static inline void
mc_rect_resize (mc_rect_t * r, gssize dw, gssize dh)
{
    r->w += dw;
    r->h += dh;
}

static inline void
mc_rect_intersect (mc_rect_t * r, const mc_rect_t * r1)
{
    gssize x, y;
    gssize x1, y1;

    /* right-down corners */
    x = r->x + (gssize) r->w;
    y = r->y + (gssize) r->h;
    x1 = r1->x + (gssize) r1->w;
    y1 = r1->y + (gssize) r1->h;
    /* right-down corner of intersection */
    x = x < x1 ? x : x1;
    y = y < y1 ? y : y1;

    /* left-up corner of intersection */
    r->x = r->x > r1->x ? r->x : r1->x;
    r->y = r->y > r1->y ? r->y : r1->y;
    /* intersection sizes */
    r->w = (gsize) (x - r->x);
    r->h = (gsize) (y - r->y);
}

static inline void
mc_rect_union (mc_rect_t * r, const mc_rect_t * r1)
{
    gssize x, y;
    gssize x1, y1;

    /* right-down corners */
    x = r->x + (gssize) r->w;
    y = r->y + (gssize) r->h;
    x1 = r1->x + (gssize) r1->w;
    y1 = r1->y + (gssize) r1->h;
    /* right-down corner of union */
    x = x > x1 ? x : x1;
    y = y > y1 ? y : y1;

    /* left-up corner of union */
    r->x = r->x < r1->x ? r->x : r1->x;
    r->y = r->y < r1->y ? r->y : r1->y;
    /* union sizes */
    r->w = (gsize) (x - r->x);
    r->h = (gsize) (y - r->y);
}

static inline gboolean
mc_rects_are_equal (const mc_rect_t * r1, const mc_rect_t * r2)
{
    return (memcmp (r1, r2, sizeof (mc_rect_t)) == 0);
}

G_END_DECLS

#endif /* MC_WIDGET_RECT_H */

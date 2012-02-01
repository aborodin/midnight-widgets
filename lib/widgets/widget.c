/* Main widget class for Midnight Commander widgets

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

#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#include <rect.h>
#include <group.h>

#include <widget.h>

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define MC_WIDGET_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MC_TYPE_WIDGET, mc_widget_private_t))

#define WIDGET_DRAG_DEFAULT WIDGET_DRAG_LIMIT_LOY

/*** file scope type declarations ****************************************************************/

struct _mc_widget_private_t
{
    mc_rect_t rect;
    mc_widget_state_t state;
    mc_widget_grow_t grow_mode;
    mc_widget_drag_t drag_mode;
    mc_widget_options_t options;
    char *event_group;
};

enum
{
    MC_WIDGET_DUMMY_PROPERTY = 0,
    MC_WIDGET_X,
    MC_WIDGET_Y,
    MC_WIDGET_W,
    MC_WIDGET_H,
};

/*** file scope variables ************************************************************************/

static const gsize shadow_size_w = 2;
static const gsize shadow_size_h = 1;

static gpointer mc_widget_parent_class = NULL;
static const mc_widget_t *target = NULL;

/*** file scope functions ************************************************************************/

static inline gsize
range (gsize val, gsize rmin, gsize rmax)
{
    if (val < rmin)
        return rmin;
    if (val > rmax)
        return rmax;
    return val;
}

/* --------------------------------------------------------------------------------------------- */

/*
    ay -- line number
    bx -- column number
    cx -- number of columns
*/

static gboolean
mc_widget_exposed_check (gboolean LAB, const mc_widget_t *view, gssize ay, gssize bx, gssize cx,
                         gssize *si)
{
    /* NOT TESTED */

    gboolean flag = FALSE;

    if (!LAB)
    {
        mc_group_t *g;
        gssize ty;
        mc_rect_t gclip;

      lab11:
        target = view;

        view = MC_WIDGET (view->owner);
        if (view == NULL)
            return TRUE;

        g = MC_GROUP (view);

        mc_widget_get_position (target, si, &ty);
        ay += ty;
        mc_group_get_clip (g, &gclip);

        if ((ay < gclip.y) || (ay >= gclip.y + (gssize) gclip.h))
            return FALSE;

        bx += *si;
        cx += *si;

        bx = MAX (bx, gclip.x);
        cx = MIN (cx, gclip.x + (gssize) gclip.w);
        if (bx >= cx)
            return FALSE;
        view = (mc_widget_t *) mc_group_get_last (g);
    }

    do
    {
        mc_rect_t vrect;

        view = (mc_widget_t *) mc_group_get_next (view->owner, view);
        if (view == target)
        {
            view = MC_WIDGET (view->owner);
            goto lab11;
        }
        if ((view->priv->state & WIDGET_STATE_VISIBLE) == 0)
            continue;

        mc_widget_get_rect (view, &vrect);
        *si = vrect.y;
        if (ay < *si)
            continue;
        *si += (gssize) vrect.h;
        if (ay >= *si)
            continue;
        *si = vrect.x;
        if (bx >= *si)
        {
            *si += (gssize) vrect.w;
            if (bx >= *si)
                continue;
            bx = *si;
            if (bx < cx)
                continue;
            return FALSE;
        }

        if (cx <= *si)
            continue;
        *si += (gssize) vrect.w;
        if (cx <= *si)
            cx = vrect.x;
        else
        {
            const mc_widget_t *retttarget = target;

            cx = vrect.x;
            flag = mc_widget_exposed_check (TRUE, view, ay, bx, cx, si);
            bx = *si;
            target = retttarget;
        }
    }
    while (!flag);

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

static inline gssize
grow (const mc_widget_t * self, gboolean is_grow_rel, gssize x, gssize s, gssize d)
{
    if (is_grow_rel)
        x = (x * s + (s - d)/2)/(s - d);
    else
        x += d;
    return x;
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_draw_under_rect (mc_widget_t * self, const mc_rect_t *r, mc_widget_t * last)
{
    mc_group_t *owner = self->owner;
    mc_rect_t clip;

    mc_group_get_clip (owner, &clip);
    mc_rect_intersect (&clip, r);
    mc_group_set_clip (owner, &clip);

    mc_group_draw_subviews (owner, mc_group_get_next_view (owner, self), last);

    clip.x = 0;
    clip.y = 0;
    mc_widget_get_size (MC_WIDGET (owner), &clip.w, &clip.h);
    mc_group_set_clip (owner, &clip);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_draw_under_view (mc_widget_t * self, gboolean doShadow, mc_widget_t * last)
{
    mc_rect_t r;

    r = self->priv->rect;
    if (doShadow)
    {
        r.w += shadow_size_w;
        r.h += shadow_size_h;
    }

    mc_widget_draw_under_rect (self, &r, last);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_draw_show (mc_widget_t * self, mc_widget_t * last)
{
    g_return_if_fail (self != NULL);
    mc_widget_draw_view (self);
    if ((self->priv->state & WIDGET_STATE_SHADOW) != 0)
        mc_widget_draw_under_view (self, TRUE, last);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_draw_hide (mc_widget_t * self, mc_widget_t * last)
{
    g_return_if_fail (self != NULL);
    mc_widget_draw_cursor (self);
    mc_widget_draw_under_view (self, (self->priv->state & WIDGET_STATE_SHADOW) != 0, last);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_get_size_limits_virtual (mc_widget_t * self, gsize *xmin, gsize *ymin,
                                                       gsize *xmax, gsize *ymax)
{
    if (xmin != NULL)
        *xmin = 0;

    if (ymin != NULL)
        *ymin = 0;

    if (self->owner != NULL)
        mc_widget_get_size (MC_WIDGET (self->owner), xmax, ymax);
    else
    {
        if (xmax != NULL)
            *xmax = G_MAXSIZE;

        if (ymax != NULL)
            *ymax = G_MAXSIZE;
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_calc_bounds_virtual (mc_widget_t * self, mc_rect_t * bounds, gssize dx, gssize dy)
{
    const mc_widget_grow_t gmode = self->priv->grow_mode;
    const gboolean is_grow_rel = ((gmode & WIDGET_GROW_REL) != 0);

    gssize x1, y1, x2, y2;              /* widget corners */
    gsize ox, oy;
    gsize xmin, ymin, xmax, ymax;      /* widget limits */

    mc_widget_get_rect (self, bounds);
    x1 = bounds->x;
    y1 = bounds->y;
    x2 = bounds->x + (gssize) bounds->w;
    y2 = bounds->y + (gssize) bounds->h;

    mc_widget_get_size (MC_WIDGET (self->owner), &ox, &oy);

    if ((gmode & WIDGET_GROW_LOX) != 0)
        x1 = grow (self, is_grow_rel, x1, ox, dx);
    if ((gmode & WIDGET_GROW_HIX) != 0)
        x2 = grow (self, is_grow_rel, x2, ox, dx);

    if ((gmode & WIDGET_GROW_LOY) != 0)
        y1 = grow (self, is_grow_rel, y1, oy, dy);
    if ((gmode & WIDGET_GROW_HIY) != 0)
        y2 = grow (self, is_grow_rel, y2, oy, dy);

    mc_widget_get_size_limits (self, &xmin, &ymin, &xmax, &ymax);

    ox = range ((gsize) (x2 - x1), xmin, xmax);
    oy = range ((gsize) (y2 - y1), ymin, ymax);

    mc_rect_init (bounds, x1, y1, ox, oy);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_change_bounds_virtual (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h)
{
    mc_widget_set_bounds (self, x, y, w, h);
    mc_widget_draw_view (self);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_set_state_virtual (mc_widget_t * self, mc_widget_state_t state, gboolean enable)
{
    /* NOT FINISHED */
    g_return_if_fail (self != NULL);

    if (enable)
        self->priv->state |= state;
    else
        self->priv->state &= ~state;

    if (self->owner == NULL)
        return;

    switch (state)
    {
    case WIDGET_STATE_VISIBLE:
        if ((self->priv->state & WIDGET_STATE_EXPOSED) != 0)
            mc_widget_set_state (self, WIDGET_STATE_EXPOSED, enable);

        if (enable)
            mc_widget_draw_show (self, NULL);
        else
            mc_widget_draw_hide (self, NULL);
        if ((self->priv->options & WIDGET_OPTION_SELECTABLE) != 0)
            mc_group_reset_current (self->owner);
        break;

    case WIDGET_STATE_CURSOR_VIS:
    case WIDGET_STATE_CURSOR_INS:
        mc_widget_draw_cursor (self);
        break;

    case WIDGET_STATE_SHADOW:
        mc_widget_draw_under_view (self, TRUE, NULL);
        break;

    case WIDGET_STATE_FOCUSED:
        mc_widget_reset_cursor (self);
#if 0
        /* TODO */
        message (MC_WIDGET (self->owner), evBroadcast, enable ? cmReceivedFocus : cmReleasedFocus,
                 self);
#endif
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_draw_virtual (mc_widget_t * self)
{
    mc_widget_t *w = MC_WIDGET (self);
    /* NOT FINISHED */
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_reset_cursor_virtual (mc_widget_t * self)
{
    mc_widget_t *w = MC_WIDGET (self);
    /* NOT FINISHED */
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_set_property (GObject * object, guint property_id, const GValue * value,
                        GParamSpec * pspec)
{
    mc_widget_t *self = MC_WIDGET (object);

    switch (property_id)
    {
    case MC_WIDGET_X:
        self->priv->rect.x = g_value_get_long (value);
        break;
    case MC_WIDGET_Y:
        self->priv->rect.y = g_value_get_long (value);
        break;
    case MC_WIDGET_W:
        self->priv->rect.w = g_value_get_ulong (value);
        break;
    case MC_WIDGET_H:
        self->priv->rect.h = g_value_get_ulong (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_finalize (GObject * object)
{
    mc_widget_t *self = MC_WIDGET (object);

    g_free (self->priv->event_group);
    G_OBJECT_CLASS (mc_widget_parent_class)->finalize (object);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_class_init (gpointer g_class, gpointer class_data)
{
    GObjectClass *o = G_OBJECT_CLASS (g_class);
    mc_widget_class_t *klass = (mc_widget_class_t *) g_class;

    (void) class_data;

    mc_widget_parent_class = g_type_class_peek_parent (klass);
    g_type_class_add_private (klass, sizeof (mc_widget_private_t));
    klass->get_size_limits = mc_widget_get_size_limits_virtual;
    klass->calc_bounds = mc_widget_calc_bounds_virtual;
    klass->change_bounds = mc_widget_change_bounds_virtual;
    klass->set_state = mc_widget_set_state_virtual;
    klass->draw = mc_widget_draw_virtual;
    klass->reset_cursor = mc_widget_reset_cursor_virtual;
    o->finalize = mc_widget_finalize;
    o->set_property = mc_widget_set_property;
    g_object_class_install_property (o, MC_WIDGET_X,
                                     g_param_spec_long ("x", "x", "x coord", 0, G_MAXSSIZE, 0,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT |
                                                        G_PARAM_PRIVATE));
    g_object_class_install_property (o, MC_WIDGET_Y,
                                     g_param_spec_long ("y", "y", "y coord", 0, G_MAXSSIZE, 0,
                                                        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT |
                                                        G_PARAM_PRIVATE));
    g_object_class_install_property (o, MC_WIDGET_W,
                                     g_param_spec_ulong ("w", "w", "widgth", 0, G_MAXSIZE, 1,
                                                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT |
                                                         G_PARAM_PRIVATE));
    g_object_class_install_property (o, MC_WIDGET_H,
                                     g_param_spec_ulong ("h", "h", "height", 0, G_MAXSIZE, 1,
                                                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT |
                                                         G_PARAM_PRIVATE));
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_widget_instance_init (GTypeInstance * instance, gpointer class)
{
    mc_widget_t *self = MC_WIDGET (instance);

    (void) class;

    self->owner = NULL;
    self->member = NULL;
    self->priv = MC_WIDGET_GET_PRIVATE (instance);
    self->priv->state = WIDGET_STATE_VISIBLE;
    self->priv->grow_mode = WIDGET_GROW_DEFAULT;
    self->priv->drag_mode = WIDGET_DRAG_DEFAULT;
    self->priv->options = WIDGET_OPTION_DEFAULT;
#if 0
    /* TODO */
    self->priv->event_group = g_strdup_printf ("%s_%p", MCEVENT_GROUP_DIALOG, (void *) self);
#else
    self->priv->event_group = NULL;
#endif
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

GType
mc_widget_get_type (void)
{
    static GType mc_widget_type_id = 0;

    if (mc_widget_type_id == 0)
    {
        static const GTypeInfo g_define_type_info =
        {
            /* interface types, classed types, instantiated types */
            .class_size = sizeof (mc_widget_class_t),
            .base_init = (GBaseInitFunc) NULL,
            .base_finalize = (GBaseFinalizeFunc) NULL,

            /* interface types, classed types, instantiated types */
            .class_init = (GClassInitFunc) mc_widget_class_init,
            .class_finalize = (GClassFinalizeFunc) NULL,
            .class_data = NULL,

            /* instantiated types */
            .instance_size = sizeof (mc_widget_t),
            .n_preallocs = 0,
            .instance_init = (GInstanceInitFunc) mc_widget_instance_init,

            /* value handling */
            .value_table = NULL
        };

        mc_widget_type_id =
            g_type_register_static (G_TYPE_OBJECT, "mc_widget_t", &g_define_type_info, 0);
    }

    return mc_widget_type_id;
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_t *
mc_widget_construct (GType object_type, gssize x, gssize y, gsize w, gsize h)
{
    return (mc_widget_t *) g_object_new (object_type, "x", x, "y", y, "w", w, "h", h,  NULL);
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_t *
mc_widget_new (void)
{
    return mc_widget_construct (MC_TYPE_WIDGET, 0, 0, 1, 1);
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_t *
mc_widget_new_with_bounds (gssize x, gssize y, gsize w, gsize h)
{
    return mc_widget_construct (MC_TYPE_WIDGET, x, y, w, h);
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_t *
mc_widget_new_with_rect (const mc_rect_t * r)
{
    if (r == NULL)
        return mc_widget_new ();
    return mc_widget_construct (MC_TYPE_WIDGET, r->x, r->y, r->w, r->h);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_delete (mc_widget_t * self)
{
    g_object_unref (G_OBJECT (self));
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_get_position (const mc_widget_t * self, gssize * x, gssize * y)
{
    g_return_if_fail (self != NULL);
    if (x != NULL)
        *x = self->priv->rect.x;
    if (y != NULL)
        *y = self->priv->rect.y;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_get_size (const mc_widget_t * self, gsize * w, gsize * h)
{
    g_return_if_fail (self != NULL);
    if (w != NULL)
        *w = self->priv->rect.w;
    if (h != NULL)
        *h = self->priv->rect.h;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_get_rect (const mc_widget_t * self, mc_rect_t * r)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (r != NULL);
    *r = self->priv->rect;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_get_size_limits (mc_widget_t * self, gsize *xmin, gsize *ymin,
                                               gsize *xmax, gsize *ymax)
{
    g_return_if_fail (self != NULL);
    MC_WIDGET_GET_CLASS (self)->get_size_limits (self, xmin, ymin, xmax, ymax);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_set_bounds (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h)
{
    g_return_if_fail (self != NULL);
    self->priv->rect.x = x;
    self->priv->rect.y = y;
    self->priv->rect.w = w;
    self->priv->rect.h = h;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_calc_bounds (mc_widget_t * self, mc_rect_t * bounds, gssize dx, gssize dy)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (bounds != NULL);
    MC_WIDGET_GET_CLASS (self)->calc_bounds (self, bounds, dx, dy);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_change_bounds (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h)
{
    g_return_if_fail (self != NULL);
    MC_WIDGET_GET_CLASS (self)->change_bounds (self, x, y, w, h);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_locate (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h)
{
    gsize xmin, ymin, xmax, ymax;       /* widget limits */
    mc_rect_t bounds, r;

    g_return_if_fail (self != NULL);

    mc_widget_get_size_limits (self, &xmin, &ymin, &xmax, &ymax);

    bounds.x = x;
    bounds.y = y;
    bounds.w = range (w, xmin, xmax);
    bounds.h = range (h, ymin, ymax);

    r = self->priv->rect;

    if (!mc_rects_are_equal (&bounds, &r))
    {
        mc_widget_change_bounds (self, bounds.x, bounds.y, bounds.w, bounds.h);

        if (self->owner != NULL && (self->priv->state & WIDGET_STATE_VISIBLE) != 0)
        {
            if ((self->priv->state & WIDGET_STATE_SHADOW) != 0)
            {
                mc_rect_union (&r, &bounds);
                r.w += shadow_size_w;
                r.h += shadow_size_h;
            }

            mc_widget_draw_under_rect (self, &r, NULL);
        }
    }
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_move_to (mc_widget_t * self, gssize x, gssize y)
{
    gsize w, h;

    g_return_if_fail (self != NULL);
    mc_widget_get_size (self, &w, &h);
    mc_widget_locate (self, x, y, w, h);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_grow_to (mc_widget_t * self, gsize w, gsize h)
{
    gsize x, y;

    g_return_if_fail (self != NULL);
    mc_widget_get_position (self, &x, &y);
    mc_widget_locate (self, x, y, w, h);
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_state_t
mc_widget_get_states (const mc_widget_t * self)
{
    g_return_val_if_fail (self != NULL, 0);
    return self->priv->state;
}

/* --------------------------------------------------------------------------------------------- */

gboolean
mc_widget_get_state (const mc_widget_t * self, mc_widget_state_t state)
{
    g_return_val_if_fail (self != NULL, FALSE);
    return ((self->priv->state & state) == state);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_set_state (mc_widget_t * self, mc_widget_state_t state, gboolean enable)
{
    g_return_if_fail (self != NULL);
    MC_WIDGET_GET_CLASS (self)->set_state (self, state, enable);
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_grow_t
mc_widget_get_grow_mode (const mc_widget_t * self)
{
    g_return_val_if_fail (self != NULL, WIDGET_GROW_DEFAULT);
    return self->priv->grow_mode;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_set_grow_mode (mc_widget_t * self, mc_widget_grow_t mode)
{
    g_return_if_fail (self != NULL);
    self->priv->grow_mode = mode;
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_drag_t
mc_widget_get_drag_mode (const mc_widget_t * self)
{
    g_return_val_if_fail (self != NULL, WIDGET_DRAG_DEFAULT);
    return self->priv->drag_mode;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_set_drag_mode (mc_widget_t * self, mc_widget_drag_t mode)
{
    g_return_if_fail (self != NULL);
    self->priv->drag_mode = mode;
}

/* --------------------------------------------------------------------------------------------- */

mc_widget_options_t
mc_widget_get_options (const mc_widget_t * self)
{
    g_return_val_if_fail (self != NULL, WIDGET_OPTION_DEFAULT);
    return self->priv->options;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_set_options (mc_widget_t * self, mc_widget_options_t options)
{
    g_return_if_fail (self != NULL);
    self->priv->options = options;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_draw (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);
    MC_WIDGET_GET_CLASS (self)->draw (self);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_draw_view (mc_widget_t * self)
{
    if (mc_widget_exposed (self))
    {
        mc_widget_draw (self);
        mc_widget_draw_cursor (self);
    }
}

/* --------------------------------------------------------------------------------------------- */

gboolean
mc_widget_exposed (const mc_widget_t * self)
{
    gsize ay;

    g_return_val_if_fail (self != NULL, FALSE);

    if ((self->priv->state & WIDGET_STATE_EXPOSED) == 0)
        return FALSE;
    if (self->priv->rect.h == 0 || self->priv->rect.w == 0)
        return FALSE;

    for (ay = 0; ay < self->priv->rect.h; ay++)
    {
        gssize si;

        if (mc_widget_exposed_check (FALSE, self, (gssize) ay, 0, (gssize) self->priv->rect.w, &si))
            return TRUE;
    }

    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_show (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);

    if ((self->priv->state & WIDGET_STATE_VISIBLE) == 0)
        mc_widget_set_state (self, WIDGET_STATE_VISIBLE, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_hide (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);

    if ((self->priv->state & WIDGET_STATE_VISIBLE) != 0)
        mc_widget_set_state (self, WIDGET_STATE_VISIBLE, FALSE);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_draw_cursor (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);

    if (((self->priv->state & WIDGET_STATE_FOCUSED) != 0) && (self->owner != NULL))
        mc_widget_reset_cursor (self);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_reset_cursor (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);
    MC_WIDGET_GET_CLASS (self)->reset_cursor (self);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_select (mc_widget_t * self)
{
    g_return_if_fail (self != NULL);

    if ((self->priv->options & WIDGET_OPTION_TOP_SELECT) != 0)
        mc_widget_make_first (self);
    else if (self->owner != NULL)
        mc_group_set_current (self->owner, self, WIDGET_SELECT_NORMAL);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_make_first (mc_widget_t * self)
{
    mc_widget_put_in_front_of (self, (mc_widget_t *) mc_group_get_first (self->owner));
}

/* --------------------------------------------------------------------------------------------- */

void
mc_widget_put_in_front_of (mc_widget_t * self, mc_widget_t * target)
{
    mc_group_t *owner;

    g_return_if_fail (self != NULL);

    owner = self->owner;

    if (owner == NULL || target == self || target == mc_group_get_next_view (owner, self) ||
        (target != NULL && target->owner != owner))
        return;

    if ((self->priv->state & WIDGET_STATE_VISIBLE) == 0)
    {
        mc_group_remove_view (owner, self);
        mc_group_insert_view (owner, self, target);
    }
    else
    {
        mc_widget_t *last_view, *p;

        last_view = (mc_widget_t *) mc_group_get_next_view (owner, self);
        p = target;

        while (p != NULL && p != self)
            p = (mc_widget_t *) mc_group_get_next_view (owner, p);
        if (p == NULL)
            last_view = target;

        self->priv->state &= ~WIDGET_STATE_VISIBLE;
        if (last_view == target)
            mc_widget_draw_hide (self, last_view);
        mc_group_remove_view (owner, self);
        mc_group_insert_view (owner, self, target);
        self->priv->state |= WIDGET_STATE_VISIBLE;
        if (last_view != target)
            mc_widget_draw_show (self, last_view);
        if ((self->priv->options & WIDGET_OPTION_SELECTABLE) != 0)
            mc_group_reset_current (owner);
    }
}

/* --------------------------------------------------------------------------------------------- */

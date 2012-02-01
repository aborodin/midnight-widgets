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
#include <widget.h>

#include <group.h>

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define MC_GROUP_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), MC_TYPE_GROUP, mc_group_private_t))

/*** file scope type declarations ****************************************************************/

struct _mc_group_private_t
{
    mc_rect_t clip;
    GQueue *widgets;
    GList *current;     /* A pointer into widgets */
};

enum
{
    MC_GROUP_DUMMY_PROPERTY
};

/*** file scope variables ************************************************************************/

static gpointer mc_group_parent_class = NULL;

/*** file scope functions ************************************************************************/

static void
mc_group_do_calc_change (gpointer data, gpointer user_data)
{
    mc_widget_t * p = (mc_widget_t *) data;
    mc_rect_t r;
    mc_rect_t *dr = (mc_rect_t *) user_data;

    mc_widget_calc_bounds (p, &r, (gssize) dr->w, (gssize) dr->h);
    mc_widget_change_bounds (p, r.x, r.y, r.w, r.h);
}

/* --------------------------------------------------------------------------------------------- */

static const mc_widget_t *
mc_group_first_match (const mc_group_t * self, mc_widget_state_t aState, mc_widget_options_t aOptions)
{
    const mc_widget_t *last;
    const mc_widget_t *temp;

    last = mc_group_get_last (self);
    if (last == NULL)
        return NULL;

    temp = last;

    while (TRUE)
    {
        if (mc_widget_get_states (temp) == aState && mc_widget_get_options (temp) == aOptions)
            return temp;

        temp = mc_group_get_next (self, temp);

        if (temp == last)
            return NULL;
    }
}

/* --------------------------------------------------------------------------------------------- */

static inline void
mc_group_focus_view (mc_group_t * self, mc_widget_t * w, gboolean enable)
{
    if (w != NULL && (mc_widget_get_states (MC_WIDGET (self)) & WIDGET_STATE_FOCUSED) != 0)
        mc_widget_set_state (w, WIDGET_STATE_FOCUSED, enable);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_group_change_bounds_virtual (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h)
{
    gsize w1, h1;

    mc_widget_get_size (self, &w1, &h1);
    mc_widget_set_bounds (self, x, y, w, h);

    if ((w1 == w) && (h1 == h))
        mc_widget_draw_view (self);
    else
    {
        mc_group_t *g = MC_GROUP (self);
        mc_rect_t r;

        mc_rect_init (&g->priv->clip, 0, 0, w, h);
        mc_rect_init (&r, 0, 0, (gsize) (w - w1), (gsize) (h - h1));
        g_queue_foreach (g->priv->widgets, mc_group_do_calc_change, &r);
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_group_reset_cursor_virtual (mc_widget_t * self)
{
    mc_group_t *g = MC_GROUP (self);

    if (g->priv->current != NULL)
        mc_widget_reset_cursor (MC_WIDGET (g->priv->current->data));
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_group_finalize (GObject *object)
{
    mc_group_t *self = MC_GROUP (object);

    mc_group_reset_current (self);
    g_queue_foreach (self->priv->widgets, (GFunc) mc_widget_delete, NULL);
    g_queue_free (self->priv->widgets);
    self->priv->current = NULL;
    G_OBJECT_CLASS (mc_group_parent_class)->finalize (object);
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_group_class_init (gpointer g_class, gpointer class_data)
{
    GObjectClass *o = G_OBJECT_CLASS (g_class);
    mc_widget_class_t *w = MC_WIDGET_CLASS (g_class);
    mc_group_class_t *klass = (mc_group_class_t *) g_class;

    (void) class_data;

    mc_group_parent_class = g_type_class_peek_parent (klass);
    g_type_class_add_private (klass, sizeof (mc_group_private_t));
    w->change_bounds = mc_group_change_bounds_virtual;
    w->reset_cursor = mc_group_reset_cursor_virtual;
    o->finalize = mc_group_finalize;
}

/* --------------------------------------------------------------------------------------------- */

static void
mc_group_instance_init (GTypeInstance * instance, gpointer class)
{
    mc_group_t *self = MC_GROUP (instance);
    gsize w, h;

    (void) class;

    self->priv = MC_GROUP_GET_PRIVATE (instance);
    self->priv->widgets = g_queue_new ();
    self->priv->current = NULL;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

GType
mc_group_get_type (void)
{
    static GType mc_group_type_id = 0;

    if (mc_group_type_id == 0)
    {
        static const GTypeInfo g_define_type_info =
        {
            /* interface types, classed types, instantiated types */
            .class_size = sizeof (mc_group_class_t),
            .base_init = (GBaseInitFunc) NULL,
            .base_finalize = (GBaseFinalizeFunc) NULL,

            /* interface types, classed types, instantiated types */
            .class_init = (GClassInitFunc) mc_group_class_init,
            .class_finalize = (GClassFinalizeFunc) NULL,
            .class_data = NULL,

            /* instantiated types */
            .instance_size = sizeof (mc_group_t),
            .n_preallocs = 0,
            .instance_init = (GInstanceInitFunc) mc_group_instance_init,

            /* value handling */
            .value_table = NULL
        };

        mc_group_type_id =
            g_type_register_static (MC_TYPE_WIDGET, "mc_group_t", &g_define_type_info, 0);
    }

    return mc_group_type_id;
}

/* --------------------------------------------------------------------------------------------- */

mc_group_t *
mc_group_construct (GType object_type, gssize x, gssize y, gsize w, gsize h)
{
    mc_group_t * self;
    mc_widget_t *widget;
    mc_widget_options_t options;

    self = (mc_group_t *) mc_widget_construct (object_type, x, y, w, h);
    widget = MC_WIDGET (self);

    options = mc_widget_get_options (widget);
    options |= WIDGET_OPTION_SELECTABLE;
    mc_widget_set_options (widget, options);

    mc_rect_init (&self->priv->clip, 0, 0, w, h);

    return self;
}

/* --------------------------------------------------------------------------------------------- */

mc_group_t *
mc_group_new (void)
{
    return mc_group_construct (MC_TYPE_GROUP, 0, 0, 1, 1);
}

/* --------------------------------------------------------------------------------------------- */

mc_group_t *
mc_group_new_with_bounds (gssize x, gssize y, gsize w, gsize h)
{
    return mc_group_construct (MC_TYPE_GROUP, x, y, w, h);
}

/* --------------------------------------------------------------------------------------------- */

mc_group_t *
mc_group_new_with_rect (const mc_rect_t * r)
{
    if (r == NULL)
        return mc_group_new ();
    return mc_group_construct (MC_TYPE_GROUP, r->x, r->y, r->w, r->h);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_delete (mc_group_t * self)
{
    g_object_unref (MC_WIDGET (self));
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_get_clip (const mc_group_t * self, mc_rect_t * clip)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (clip != NULL);
    *clip = self->priv->clip;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_set_clip (mc_group_t * self, const mc_rect_t * clip)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (clip != NULL);
    self->priv->clip = *clip;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_set_current (mc_group_t * self, mc_widget_t * w, mc_widget_select_mode_t mode)
{
    g_return_if_fail (self != NULL);

    if (w != NULL && self->priv->current != w->member)
    {
        mc_widget_t *wcurrent = MC_WIDGET (self->priv->current->data);

        mc_group_focus_view (self, wcurrent, FALSE);

        /* Test if focus lost was allowed and focus has really been loose */
        if (mode != WIDGET_SELECT_NORMAL || self->priv->current == NULL ||
            (mc_widget_get_states (wcurrent) & WIDGET_STATE_FOCUSED) == 0)
        {
            if (mode != WIDGET_SELECT_ENTER && self->priv->current != NULL)
                mc_widget_set_state (wcurrent, WIDGET_STATE_SELECTED, FALSE);
            if (mode != WIDGET_SELECT_LEAVE)
                mc_widget_set_state (w, WIDGET_STATE_SELECTED, TRUE);
            mc_group_focus_view (self, w, TRUE);
            self->priv->current = w->member;
        }
    }
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_current (const mc_group_t * self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return MC_WIDGET (self->priv->current->data);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_reset_current (mc_group_t * self)
{
    mc_group_set_current (self,
                          (mc_widget_t *) mc_group_first_match (self, WIDGET_STATE_VISIBLE,
                                                                WIDGET_OPTION_SELECTABLE),
                          WIDGET_SELECT_NORMAL);
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_first (const mc_group_t * self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return MC_WIDGET (g_queue_peek_tail (self->priv->widgets));
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_last (const mc_group_t * self)
{
    g_return_val_if_fail (self != NULL, NULL);
    return MC_WIDGET (g_queue_peek_head (self->priv->widgets));
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_next (const mc_group_t * self, const mc_widget_t *w)
{
    GList *next;

    g_return_val_if_fail (self != NULL, NULL);

    next = g_list_previous (w->member);
    if (next == NULL)
        next = g_queue_peek_tail_link (self->priv->widgets);

    return MC_WIDGET (next->data);
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_previous (const mc_group_t * self, const mc_widget_t *w)
{
    GList *prev;

    g_return_val_if_fail (self != NULL, NULL);

    prev = g_list_next (w->member);
    if (prev == NULL)
        prev = g_queue_peek_head_link (self->priv->widgets);

    return MC_WIDGET (prev->data);
}

/* --------------------------------------------------------------------------------------------- */

const GList *
mc_group_get_member (const mc_group_t * self, const mc_widget_t * w)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (self->priv->widgets != NULL, NULL);

    return g_queue_find (self->priv->widgets, w);
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_next_view (const mc_group_t * self, const mc_widget_t *w)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (w != NULL, NULL);
    g_return_val_if_fail (self == w->owner, NULL);

    if (w == mc_group_get_last (self))
        return NULL;

    return mc_group_get_next (self, w);
}

/* --------------------------------------------------------------------------------------------- */

const mc_widget_t *
mc_group_get_previous_view (const mc_group_t * self, const mc_widget_t *w)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (w != NULL, NULL);
    g_return_val_if_fail (self == w->owner, NULL);

    if (w == mc_group_get_first (self))
        return NULL;

    return mc_group_get_previous (self, w);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_insert (mc_group_t * self, mc_widget_t * w)
{
    mc_group_insert_before (self, w, (mc_widget_t *) mc_group_get_first (self));
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_insert_before (mc_group_t * self, mc_widget_t * w, mc_widget_t * before)
{
    mc_widget_state_t save_states;
    mc_widget_options_t save_options;
    mc_rect_t r;
    gsize sw, sh;

    g_return_if_fail (self != NULL);

    /* can't insert nothing */
    g_return_if_fail (w != NULL);
    /* widget is already in some group */
    g_return_if_fail (w->owner == NULL);
    /* 'before' should be contained in this group */
    g_return_if_fail ((before == NULL) || (before->owner == self));

    save_states = mc_widget_get_states (w);
    save_options = mc_widget_get_options (w);

    mc_widget_get_rect (w, &r);
    mc_widget_get_size (MC_WIDGET (self), &sw, &sh);
    if ((save_options & WIDGET_OPTION_CENTER_X) != 0)
        r.x = (sw - r.w) / 2;
    if ((save_options & WIDGET_OPTION_CENTER_Y) != 0)
        r.y = (sh - r.h) / 2;

    mc_widget_set_bounds (w, r.x, r.y, r.w, r.h);
    mc_widget_hide (w);
    mc_group_insert_view (self, w, before);
    if ((save_states & WIDGET_STATE_VISIBLE) != 0)
        mc_widget_show (w);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_insert_view (mc_group_t * self, mc_widget_t * w, mc_widget_t * before)
{
    g_return_if_fail (self != NULL);

    w->owner = self;

    if (before == NULL || g_queue_is_empty (self->priv->widgets))
        g_queue_push_tail (self->priv->widgets, w);
    else
        g_queue_insert_after (self->priv->widgets, before->member, w);

    w->member = g_queue_find (self->priv->widgets, w);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_remove (mc_group_t *self, mc_widget_t * w)
{
    mc_widget_state_t save_state;

    g_return_if_fail (self != NULL);
    g_return_if_fail (w != NULL);
    g_return_if_fail (w->owner == self);

    save_state = mc_widget_get_states (w);
    mc_widget_hide (w);
    mc_group_remove_view (self, w);
    w->owner = NULL;
    if ((save_state & WIDGET_STATE_VISIBLE) != 0)
        mc_widget_show (w);
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_remove_view (mc_group_t *self, mc_widget_t * w)
{
    if (mc_group_get_last (self) != NULL)
    {
        g_queue_remove (self->priv->widgets, w);
        w->member = NULL;
    }
}

/* --------------------------------------------------------------------------------------------- */

void
mc_group_draw_subviews (mc_group_t * self, const mc_widget_t * w, mc_widget_t * bottom)
{
    for (; w != bottom; w = mc_group_get_next_view (self, w))
        mc_widget_draw_view ((mc_widget_t *) w);
}

/* --------------------------------------------------------------------------------------------- */

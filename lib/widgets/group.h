
#ifndef MC_WIDGET_GROUP_H
#define MC_WIDGET_GROUP_H

#include <glib.h>

#include <widget.h>

G_BEGIN_DECLS

/*** typedefs (not structures) and defined constants *********************************************/

#define MC_TYPE_GROUP (mc_group_get_type ())
#define MC_GROUP(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MC_TYPE_GROUP, mc_group_t))
#define MC_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MC_TYPE_GROUP, mc_group_class))
#define MC_IS_GROUP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MC_TYPE_GROUP))
#define MC_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MC_TYPE_GROUP))
#define MC_GROUP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MC_TYPE_GROUP, mc_group_class))

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures) ****************************************/

typedef struct _mc_group_t mc_group_t;
typedef struct _mc_group_class_t mc_group_class_t;
typedef struct _mc_group_private_t mc_group_private_t;

struct _mc_group_t
{
    mc_widget_t parent_instance;
    mc_group_private_t *priv;
};

struct _mc_group_class_t
{
    mc_widget_class_t parent_class;
};

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

GType mc_group_get_type (void);

mc_group_t *mc_group_construct (GType object_type, gssize x, gssize y, gsize w, gsize h);
mc_group_t *mc_group_new (void);
mc_group_t *mc_group_new_with_bounds (gssize x, gssize y, gsize w, gsize h);
mc_group_t *mc_group_new_with_rect (const mc_rect_t * r);
void mc_group_delete (mc_group_t * self);

void mc_group_get_clip (const mc_group_t * self, mc_rect_t * clip);
void mc_group_set_clip (mc_group_t * self, const mc_rect_t * clip);

void mc_group_set_current (mc_group_t * self, mc_widget_t * w, mc_widget_select_mode_t mode);
const mc_widget_t *mc_group_get_current (const mc_group_t * self);
void mc_group_reset_current (mc_group_t * self);
const mc_widget_t *mc_group_get_first (const mc_group_t * self);
const mc_widget_t *mc_group_get_last (const mc_group_t * self);
const mc_widget_t *mc_group_get_next (const mc_group_t * self, const mc_widget_t *w);
const mc_widget_t *mc_group_get_previous (const mc_group_t * self, const mc_widget_t *w);
const GList *mc_group_get_member (const mc_group_t * self, const mc_widget_t * w);

const mc_widget_t *mc_group_get_next_view (const mc_group_t * self, const mc_widget_t *w);
const mc_widget_t *mc_group_get_previous_view (const mc_group_t * self, const mc_widget_t *w);

void mc_group_insert (mc_group_t * self, mc_widget_t * w);
void mc_group_insert_before (mc_group_t * self, mc_widget_t * w, mc_widget_t * before);
void mc_group_insert_view (mc_group_t * self, mc_widget_t * w, mc_widget_t * before);

void mc_group_remove (mc_group_t * self, mc_widget_t * w);
void mc_group_remove_view (mc_group_t * self, mc_widget_t * w);

void mc_group_draw_subviews (mc_group_t * self, const mc_widget_t * w, mc_widget_t * bottom);

/*** inline functions ****************************************************************************/

G_END_DECLS

#endif /* MC_WIDGET_GROUP_H */

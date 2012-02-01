
#ifndef MC_WIDGET_WIDGET_H
#define MC_WIDGET_WIDGET_H

#include <glib.h>
#include <glib-object.h>

#include <rect.h>

G_BEGIN_DECLS

/*** typedefs (not structures) and defined constants *********************************************/

#define MC_TYPE_WIDGET (mc_widget_get_type ())
#define MC_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MC_TYPE_WIDGET, mc_widget_t))
#define MC_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MC_TYPE_WIDGET, mc_widget_class_t))
#define MC_IS_WIDGET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MC_TYPE_WIDGET))
#define MC_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MC_TYPE_WIDGET))
#define MC_WIDGET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MC_TYPE_WIDGET, mc_widget_class_t))

/*** enums ***************************************************************************************/

/**
 * mc_widget_grow_t:
 * @WIDGET_GROW_LOX: distance between left boundary of widget and left boundary of owner is constant.
 * @WIDGET_GROW_LOY: distance between top boundary of widget and top boundary of owner is constant.
 * @WIDGET_GROW_HIX: distance between right boundary of widget and right boundary of owner is constant.
 * @WIDGET_GROW_HIY: distance between bottom boundary of widget and bottom boundary of owner is constant.
 * @WIDGET_GROW_ALL: sizes of widget are constant and widget is moved with right-bottom corner of widget.
 * @WIDGET_GROW_REL: widget tryes change it sizes relative to owner sizes.
 *
 * Describe the behaviour of the widget when its owner's size is changed.
 **/

typedef enum
{
    WIDGET_GROW_DEFAULT = 0x0,
    WIDGET_GROW_LOX = 0x01,
    WIDGET_GROW_LOY = 0x02,
    WIDGET_GROW_HIX = 0x04,
    WIDGET_GROW_HIY = 0x08,
    WIDGET_GROW_ALL = WIDGET_GROW_LOX | WIDGET_GROW_LOY | WIDGET_GROW_HIX | WIDGET_GROW_HIY,
    WIDGET_GROW_REL = 0x10
} mc_widget_grow_t;

typedef enum
{
    WIDGET_DRAG_MOVE = 0x01,
    WIDGET_DRAG_GROW = 0x02,
    WIDGET_DRAG_LIMIT_LOX = 0x10,
    WIDGET_DRAG_LIMIT_LOY = 0x20,
    WIDGET_DRAG_LIMIT_HIX = 0x40,
    WIDGET_DRAG_LIMIT_HIY = 0x80,
    WIDGET_DRAG_LIMIT_ALL = WIDGET_DRAG_LIMIT_LOX | WIDGET_DRAG_LIMIT_LOY |
                            WIDGET_DRAG_LIMIT_HIX | WIDGET_DRAG_LIMIT_HIY
} mc_widget_drag_t;

typedef enum
{
    WIDGET_STATE_VISIBLE    = 0x001,
    WIDGET_STATE_CURSOR_VIS = 0x002,
    WIDGET_STATE_CURSOR_INS = 0x004,
    WIDGET_STATE_SHADOW     = 0x008,
    WIDGET_STATE_ACTIVE     = 0x010,
    WIDGET_STATE_SELECTED   = 0x020,
    WIDGET_STATE_FOCUSED    = 0x040,
    WIDGET_STATE_DRAGGING   = 0x080,
    WIDGET_STATE_DISABLED   = 0x100,
    WIDGET_STATE_MODAL      = 0x200,
    WIDGET_STATE_DEFAULT    = 0x400,
    WIDGET_STATE_EXPOSED    = 0x800
} mc_widget_state_t;

typedef enum
{
    WIDGET_OPTION_DEFAULT      = 0x0,
    WIDGET_OPTION_SELECTABLE   = 0x001,
    WIDGET_OPTION_TOP_SELECT   = 0x002,
    WIDGET_OPTION_FIRST_CLICK  = 0x004,
    WIDGET_OPTION_FRAMED       = 0x008,
    WIDGET_OPTION_PRE_PROCESS  = 0x010,
    WIDGET_OPTION_POST_PROCESS = 0x020,
    WIDGET_OPTION_BUFFERED     = 0x040,
    WIDGET_OPTION_TILEABLE     = 0x080,
    WIDGET_OPTION_CENTER_X     = 0x100,
    WIDGET_OPTION_CENTER_Y     = 0x200,
    WIDGET_OPTION_CENTERED     = 0x300
} mc_widget_options_t;

typedef enum
{
    WIDGET_SELECT_NORMAL = 0,
    WIDGET_SELECT_ENTER,
    WIDGET_SELECT_LEAVE
} mc_widget_select_mode_t;

/*** structures declarations (and typedefs of structures) ****************************************/

typedef struct _mc_widget_t mc_widget_t;
typedef struct _mc_widget_class_t mc_widget_class_t;
typedef struct _mc_widget_private_t mc_widget_private_t;

struct _mc_group_t;

struct _mc_widget_t
{
    GObject parent_instance;

    mc_widget_private_t *priv;
    struct _mc_group_t *owner;
    GList *member; /* pointer to this in owner */
};

struct _mc_widget_class_t
{
    GObjectClass parent_class;

    void (*get_size_limits) (mc_widget_t * self, gsize *xmin, gsize *ymin, gsize *xmax, gsize *ymax);
    void (*calc_bounds) (mc_widget_t * self, mc_rect_t *bounds, gssize dx, gssize dy);
    void (*change_bounds) (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h);
    void (*set_state) (mc_widget_t * self, mc_widget_state_t state, gboolean enable);
    void (*draw) (mc_widget_t *self);
    void (*reset_cursor) (mc_widget_t * self);
};

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

GType mc_widget_get_type (void);

mc_widget_t *mc_widget_construct (GType object_type, gssize x, gssize y, gsize w, gsize h);
mc_widget_t *mc_widget_new (void);
mc_widget_t *mc_widget_new_with_bounds (gssize x, gssize y, gsize w, gsize h);
mc_widget_t *mc_widget_new_with_rect (const mc_rect_t * r);
void mc_widget_delete (mc_widget_t * self);

void mc_widget_get_position (const mc_widget_t * self, gssize * x, gssize * y);
void mc_widget_get_size (const mc_widget_t * self, gsize * w, gsize * h);
void mc_widget_get_rect (const mc_widget_t * self, mc_rect_t * r);
void mc_widget_get_size_limits (mc_widget_t * self, gsize *xmin, gsize *ymin, gsize *xmax, gsize *ymax);

void mc_widget_set_bounds (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h);
void mc_widget_calc_bounds (mc_widget_t * self, mc_rect_t * bounds, gssize dx, gssize dy);
void mc_widget_change_bounds (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h);
void mc_widget_locate (mc_widget_t * self, gssize x, gssize y, gsize w, gsize h);

void mc_widget_move_to (mc_widget_t * self, gssize x, gssize y);
void mc_widget_grow_to (mc_widget_t * self, gsize w, gsize h);

mc_widget_state_t mc_widget_get_states (const mc_widget_t * self);
gboolean mc_widget_get_state (const mc_widget_t * self, mc_widget_state_t state);
void mc_widget_set_state (mc_widget_t * self, mc_widget_state_t state, gboolean enable);

mc_widget_grow_t mc_widget_get_grow_mode (const mc_widget_t * self);
void mc_widget_set_grow_mode (mc_widget_t * self, mc_widget_grow_t mode);

mc_widget_drag_t mc_widget_get_drag_mode (const mc_widget_t * self);
void mc_widget_set_drag_mode (mc_widget_t * self, mc_widget_drag_t mode);

mc_widget_options_t mc_widget_get_options (const mc_widget_t * self);
void mc_widget_set_options (mc_widget_t * self, mc_widget_options_t options);

void mc_widget_draw (mc_widget_t * self);
void mc_widget_draw_view (mc_widget_t * self);
gboolean mc_widget_exposed (const mc_widget_t * self);

void mc_widget_show (mc_widget_t * self);
void mc_widget_hide (mc_widget_t * self);

void mc_widget_draw_cursor (mc_widget_t * self);
void mc_widget_reset_cursor (mc_widget_t * self);

void mc_widget_select (mc_widget_t * self);
void mc_widget_make_first (mc_widget_t * self);
void mc_widget_put_in_front_of (mc_widget_t * self, mc_widget_t * target);

/*** inline functions ****************************************************************************/

G_END_DECLS

#endif /* MC_WIDGET_WIDGET_H */

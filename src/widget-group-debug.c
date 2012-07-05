
#include <config.h>

#include <stdlib.h>
#include <stdio.h>

#include <widget.h>
#include <group.h>


int
main (int argc, char *argv[])
{
    mc_group_t *g;
    mc_widget_t *w1, *w2;
    mc_rect_t r;

    g_type_init_with_debug_flags (G_TYPE_DEBUG_OBJECTS);

    w1 = mc_widget_new_with_bounds (3, 3, 5, 4);
    mc_widget_set_state (w1, WIDGET_STATE_EXPOSED, TRUE);
    mc_widget_get_rect (w1, &r);
    if (!(r.x == 3 && r.y == 3 && r.w == 5 && r.h == 4))
    {
        fprintf (stderr, "%s\n", "Bounds test #1 failure!");
        return 1;
    }

    g = mc_group_new_with_bounds (0, 0, 80, 25);
    mc_widget_get_rect (MC_WIDGET (g), &r);
    if (!(r.x == 0 && r.y == 0 && r.w == 80 && r.h == 25))
    {
        fprintf (stderr, "%s\n", "Bounds test #2 failure!");
        return 1;
    }

    mc_widget_set_state (MC_WIDGET (g), WIDGET_STATE_EXPOSED, TRUE);
    mc_widget_set_state (MC_WIDGET (g), WIDGET_STATE_VISIBLE, TRUE);

    mc_group_insert (g, w1);

    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #1 failure!");
        return 1;
    }

    mc_widget_set_state (w1, WIDGET_STATE_VISIBLE, FALSE);
    if (mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #2 failure!");
        return 1;
    }

    mc_widget_set_state (w1, WIDGET_STATE_EXPOSED, TRUE);
    mc_widget_set_state (w1, WIDGET_STATE_VISIBLE, TRUE);

    w2 = mc_widget_new_with_bounds (10, 10, 5, 4);
    mc_widget_set_state (w2, WIDGET_STATE_EXPOSED, TRUE);
    mc_group_insert (g, w2);

    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #3 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #4 failure!");
        return 1;
    }

    mc_widget_move_to (w2, 3, 3);
    mc_widget_get_rect (w2, &r);
    if (!(r.x == 3 && r.y == 3 && r.w == 5 && r.h == 4))
    {
        fprintf (stderr, "%s\n", "Bounds test #3 failure!");
        return 1;
    }

    if (mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #5 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #6 failure!");
        return 1;
    }

    mc_widget_make_first (w1);
    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #6 failure!");
        return 1;
    }
    if (mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #7 failure!");
        return 1;
    }

    mc_widget_move_to (w1, 3, 4);
    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #8 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #9 failure!");
        return 1;
    }

    mc_widget_move_to (w1, 4, 3);
    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #10 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #11 failure!");
        return 1;
    }

    mc_widget_move_to (w1, 2, 3);
    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #12 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #13 failure!");
        return 1;
    }

    mc_widget_move_to (w1, 3, 2);
    if (!mc_widget_exposed (w1))
    {
        fprintf (stderr, "%s\n", "Visibility test #14 failure!");
        return 1;
    }
    if (!mc_widget_exposed (w2))
    {
        fprintf (stderr, "%s\n", "Visibility test #15 failure!");
        return 1;
    }

    mc_group_delete (g);

    return 0;
}

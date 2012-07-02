/*
   libmc - check base capabilities of widget and group

   Copyright (C) 2012
   The Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <slavazanko@gmail.com>, 2012

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

#define TEST_SUITE_NAME "lib/widgets"

#include <config.h>

#include <check.h>

#include <widget.h>
#include <group.h>

static void
setup (void)
{
    g_type_init_with_debug_flags (G_TYPE_DEBUG_OBJECTS);
}

static void
teardown (void)
{
}

/* --------------------------------------------------------------------------------------------- */

START_TEST (widget_bounds)
{
    mc_widget_t *w;
    mc_rect_t r;

    w = mc_widget_new_with_bounds (1, 2, 3, 4);

    mc_widget_get_rect (w, &r);
    fail_unless (r.x == 1 && r.y == 2 && r.w == 3 && r.h == 4, "%s\n", "Bounds test #1 failed!");

    mc_widget_change_bounds (w, 10, 20, 30, 40);
    mc_widget_get_rect (w, &r);
    fail_unless (r.x == 10 && r.y == 20 && r.w == 30 && r.h == 40, "%s\n", "Bounds test #2 failed!");

    mc_widget_delete (w);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

START_TEST (group_bounds)
{
    mc_group_t *g;
    mc_widget_t *w;
    mc_rect_t r;

    g = mc_group_new_with_bounds (1, 2, 3, 4);
    w = MC_WIDGET (g);

    mc_widget_get_rect (w, &r);
    fail_unless (r.x == 1 && r.y == 2 && r.w == 3 && r.h == 4, "%s\n", "Bounds test #3 failed!");

    mc_widget_change_bounds (w, 10, 20, 30, 40);
    mc_widget_get_rect (w, &r);
    fail_unless (r.x == 10 && r.y == 20 && r.w == 30 && r.h == 40, "%s\n", "Bounds test #4 failed!");

    mc_group_delete (g);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

START_TEST (widget_visibility)
{
    mc_group_t *g;
    mc_widget_t *gw;
    mc_widget_t *w1, *w2;
    mc_rect_t r;

    g = mc_group_new_with_bounds (0, 0, 25, 80);
    gw = MC_WIDGET (g);

    mc_widget_set_state (gw, WIDGET_STATE_EXPOSED, TRUE);
    fail_unless (mc_widget_exposed (gw), "%s\n", "Visibility test #1 failed!");

    w1 = mc_widget_new_with_bounds (5, 5, 7, 8);
    mc_widget_set_state (w1, WIDGET_STATE_EXPOSED, TRUE);
    mc_group_insert (g, w1);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #2 failed!");

    w2 = mc_widget_new_with_bounds (15, 15, 7, 8);
    mc_widget_set_state (w2, WIDGET_STATE_EXPOSED, TRUE);
    mc_group_insert (g, w2);
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #3 failed!");

    mc_widget_move_to (w2, 5, 5);
    fail_unless (!mc_widget_exposed (w1), "%s\n", "Visibility test #4 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #5 failed!");

    mc_widget_make_first (w1);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #6 failed!");
    fail_unless (!mc_widget_exposed (w2), "%s\n", "Visibility test #7 failed!");

    mc_widget_move_to (w1, 3, 4);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #8 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #9 failed!");

    mc_widget_move_to (w1, 4, 3);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #10 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #11 failed!");

    mc_widget_move_to (w1, 2, 3);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #12 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #13 failed!");

    mc_widget_move_to (w1, 3, 2);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #14 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #15 failed!");

    mc_widget_move_to (w2, 10, 20);
    fail_unless (mc_widget_exposed (w1), "%s\n", "Visibility test #16 failed!");
    fail_unless (mc_widget_exposed (w2), "%s\n", "Visibility test #17 failed!");

    mc_group_delete (g);
}
END_TEST

/* --------------------------------------------------------------------------------------------- */

int
main (void)
{
    int number_failed;

    Suite *s = suite_create (TEST_SUITE_NAME);
    TCase *tc_core = tcase_create ("Core");
    SRunner *sr;

    tcase_add_checked_fixture (tc_core, setup, teardown);

    /* Add new tests here: *************** */
    tcase_add_test (tc_core, widget_bounds);
    tcase_add_test (tc_core, group_bounds);
    tcase_add_test (tc_core, widget_visibility);
    /* *********************************** */

    suite_add_tcase (s, tc_core);
    sr = srunner_create (s);
    srunner_set_log (sr, "widget_group.log");
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? 0 : 1;
}

/* --------------------------------------------------------------------------------------------- */

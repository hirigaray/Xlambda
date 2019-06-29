/* see LICENSES.md for details on the licensing of this software */
#include <stdlib.h>
#include <stdio.h>

#include <libguile.h>
#include <xcb/xcb.h>

/*
 * Variables used to hold the connection to the X server,
 * and the first screen of this connection.
 */
xcb_connection_t *conn;
xcb_screen_t     *scrn;

/* Function prototypes */
static SCM wm_list_windows();
static SCM wm_exists_p(SCM wid);
static SCM wm_get_focused_window();
static SCM wm_set_focused_window(SCM s_wid);
static SCM wm_set_border(SCM s_wid, SCM s_width, SCM s_color);
static SCM wm_get_cursor_pos();
static SCM wm_set_cursor_pos(SCM s_x, SCM s_y);
static SCM wm_teleport(SCM wid, SCM x, SCM y, SCM width, SCM s_height);

static SCM wm_window_map(SCM s_wid, SCM s_mode);
static SCM wm_window_unmap(SCM s_wid, SCM s_mode);

static SCM wm_window_x(SCM s_wid);
static SCM wm_window_y(SCM s_wid);
static SCM wm_window_w(SCM s_wid);
static SCM wm_window_h(SCM s_wid);
static SCM wm_window_b(SCM s_wid);
static SCM wm_window_m(SCM s_wid);
static SCM wm_window_i(SCM s_wid);

static void*
register_functions (void* data)
{
/* scheme-name
 * args: required, optional, rest-list,
 * c_name */
	scm_c_define_gsubr("wm/window/x",        1, 0, 0, &wm_window_x);
	scm_c_define_gsubr("wm/window/y",        1, 0, 0, &wm_window_y);
	scm_c_define_gsubr("wm/window/w",        1, 0, 0, &wm_window_w);
	scm_c_define_gsubr("wm/window/h",        1, 0, 0, &wm_window_h);
	scm_c_define_gsubr("wm/window/b",        1, 0, 0, &wm_window_b);
	scm_c_define_gsubr("wm/window/m",        1, 0, 0, &wm_window_m);
	scm_c_define_gsubr("wm/window/i",        1, 0, 0, &wm_window_i);
	scm_c_define_gsubr("wm/window/map",      1, 0, 0, &wm_set_focused_window);
	scm_c_define_gsubr("wm/window/unmap",    1, 0, 0, &wm_set_focused_window);
	scm_c_define_gsubr("wm/get-focused",     0, 0, 0, &wm_get_focused_window);
	scm_c_define_gsubr("wm/set-focused",     1, 0, 0, &wm_set_focused_window);
	scm_c_define_gsubr("wm/get-cursor-pos",  0, 0, 0, &wm_get_cursor_pos);
	scm_c_define_gsubr("wm/set-cursor-pos!", 2, 0, 0, &wm_set_cursor_pos);
	scm_c_define_gsubr("wm/list-windows",    0, 0, 0, &wm_list_windows);
	scm_c_define_gsubr("wm/exists?",         1, 0, 0, &wm_exists_p);
	scm_c_define_gsubr("wm/teleport!",       5, 0, 0, &wm_teleport);
	return NULL;
}

int
main(int argc, char** argv)
{
	/* Connect to X */
	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn))
		return 0;

	/* Get the first screen */
	scrn = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if (scrn == NULL)
		return 0;

	/* Start Guile */
	scm_with_guile(&register_functions, NULL);
	scm_shell(argc, argv);

	/* When it's done, quit cleanly */
	if (conn) {
		xcb_disconnect(conn);
		return 0;
	}
}

/*
 * Returns the list of all existing windows 
 * The windows are listed in stacking order, from lower to upper window.
 */
static SCM
wm_list_windows()
{
	xcb_window_t x_window;
	/* Initialize the window list as a Guile empty list */
	SCM s_window_list = SCM_EOL;

	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(conn, scrn->root);
	r = xcb_query_tree_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(1);

	xcb_window_t *x_window_list = xcb_query_tree_children(r);

	/* wi = window index */
	for (int wi = 0; wi <= r->children_len; wi++) {
		x_window = x_window_list[wi];
		SCM s_window = scm_from_uint32(x_window);
		/* Put the window ID into a list */
		SCM s_wid = scm_list_1(s_window);
		/* Append the list with the wid to the total list */ 
		s_window_list = scm_append(scm_list_2 (s_window_list, s_wid));
	}

	free(r);
	return s_window_list;
}

/* Check existence of a window. */
static SCM
wm_exists_p(SCM wid)
{
	xcb_window_t w = scm_to_uint32(wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, w);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return scm_from_bool(0);

	free(r);
	return scm_from_bool(1);
}

/* Teleport a window to the given position. */
static SCM
wm_teleport(SCM wid, SCM x, SCM y, SCM width, SCM height)
{
	xcb_window_t w = scm_to_uint32(wid);
	uint32_t values[4];
	uint32_t mask =   XCB_CONFIG_WINDOW_X
	                | XCB_CONFIG_WINDOW_Y
	                | XCB_CONFIG_WINDOW_WIDTH
	                | XCB_CONFIG_WINDOW_HEIGHT;
	values[0] = scm_to_uint32(x);
	values[1] = scm_to_uint32(y);
	values[2] = scm_to_uint32(width);
	values[3] = scm_to_uint32(height);
	xcb_configure_window(conn, w, mask, values);

	xcb_flush(conn);
	return scm_from_bool(1);
	/* todo: return old geometry, maybe on version 0.2, so things like
	fullscreening/returning to old position are easier */
}


/* Get focused window identifier */
static SCM
wm_get_focused_window()
{
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(1);

	SCM fwid = scm_from_uint32(r->focus);
	free(r);

	return fwid;
}

/* Return window's x offset position */
static SCM
wm_window_x(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM x = scm_from_int(r->x);
	free(r);
	return x;
}

/* Return window's y offset position */
static SCM
wm_window_y(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM y = scm_from_int(r->y);
	free(r);
	return y;
}

/* Return window's width */
static SCM
wm_window_w(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM w = scm_from_int(r->width);
	free(r);
	return w;
}

/* Return window's height */
static SCM
wm_window_h(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM h = scm_from_int(r->height);
	free(r);
	return h;
}

/* Return window's border width */
static SCM
wm_window_b(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM b = scm_from_int(r->border_width);
	free(r);
	return b;
}

/* Return window's mapped status */
static SCM
wm_window_m(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM m = scm_from_int(r->map_state);
	free(r);
	return m;
}

/* Return whether a window should be ignored or not */
static SCM
wm_window_i(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_BOOL_F; /* this is probably the root window
                           * in any case, it shouldn't be handled. */

	/* override_redirect is 1 when true, and 2 when false */
	SCM i;
	switch (r->override_redirect) {
	case 0:
		i = SCM_BOOL_T;
	case 1:
		i = SCM_BOOL_F;
	}

	free(r);
	return i;
}

/* Get the cursor position, and return its coordinates as (x y) */
static SCM
wm_get_cursor_pos()
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, scrn->root);
	r = xcb_query_pointer_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(0);

	SCM xypos;

	if (r->child != XCB_NONE) {
		xypos = scm_list_2(scm_from_uint16(r->win_x), scm_from_uint16(r->win_y));
	} else {
		xypos = scm_list_2(scm_from_uint16(r->root_x), scm_from_uint16(r->root_y));
	}

	free(r);
	return xypos;
}


/* Set a window's border.
 * The color should be a hexadecimal number, eg: "#xffffff" */
static SCM
wm_set_border(SCM s_wid, SCM s_width, SCM s_color)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	int width = scm_to_int32(s_wid);
	int color = scm_to_int32(s_color);

	uint32_t values[1];
	int mask, retval = 0;

	/* change width if > 0 */
	if (width > -1) {
		values[0] = width;
		mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
		xcb_configure_window(conn, wid, mask, values);
		retval++;
	}

	/* change color if > 0 */
	if (color > -1) {
		values[0] = color;
		mask = XCB_CW_BORDER_PIXEL;
		xcb_change_window_attributes(conn, wid, mask, values);
		retval++;
	}

	xcb_flush(conn);
	return scm_from_int(retval);
}

/* Change the cursor position. */
static SCM
wm_set_cursor_pos(SCM s_x, SCM s_y)
{
	int x = scm_to_int(s_x);
	int y = scm_to_int(s_y);
	xcb_warp_pointer(conn, XCB_NONE, scrn->root, 0, 0, 0, 0, x, y);
	return scm_from_bool(1);
}

static SCM
wm_set_override(SCM s_wid, SCM s_mode)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	int mode = scm_to_int(s_mode);
	
	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { mode };

	xcb_change_window_attributes(conn, wid, mask, val);

	return scm_from_bool(1);
}

/* Map a window */
static SCM
wm_window_map(SCM s_wid, SCM s_mode)
{
 	xcb_window_t wid = scm_to_uint32(s_wid);
	xcb_map_window(conn, wid);
	xcb_flush(conn);
	return scm_list_2(s_wid, SCM_BOOL_T);
}

/* Unmap a window */
static SCM
wm_window_unmap(SCM s_wid, SCM s_mode)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	xcb_unmap_window(conn, wid);
	xcb_flush(conn);
	return scm_list_2(s_wid, SCM_BOOL_T);
}

/* Give the input focus to the specified window */
static SCM
wm_set_focused_window(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	/* Bring window to the front */
	xcb_configure_window(conn, wid, XCB_STACK_MODE_ABOVE, 0);
	xcb_flush(conn);
	return scm_from_bool(1);
}

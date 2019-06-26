/* see LICENSES.md for details on the licensing of this software */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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
static SCM wm_listable_p(SCM s_wid, SCM s_mask);
static SCM wm_get_focused_window();
static SCM wm_set_focused_window(SCM s_wid);
static SCM wm_get_attribute(SCM s_wid, SCM s_attr);
static SCM wm_set_border(SCM s_wid, SCM s_width, SCM s_color);
static SCM wm_get_cursor(SCM s_wid);
static SCM wm_set_cursor(SCM s_x, SCM s_y, SCM s_mode);
static SCM wm_teleport(SCM wid, SCM x, SCM y, SCM width, SCM s_height);
static SCM wm_remap(SCM wid, SCM mode);

static void*
register_functions (void* data)
{
/* scheme-name
 * args: required, optional, rest-list,
 * c_name */
	scm_c_define_gsubr("wm/get-focused",
		0, 0, 0, &wm_get_focused_window);
	scm_c_define_gsubr("wm/set-focused",
		1, 0, 0, &wm_set_focused_window);
	scm_c_define_gsubr("wm/list-windows",
		0, 0, 0, &wm_list_windows);
	scm_c_define_gsubr("wm/exists?",
		1, 0, 0, &wm_exists_p);
	scm_c_define_gsubr("wm/teleport!",
		5, 0, 0, &wm_teleport);
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
	xcb_window_t w = 0;
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return scm_from_bool(1);

	w = r->focus;
	free(r);

	return scm_from_uint32(w);
}


/*
 * Retrieve the value of an attribute for a specific windows.
 * The possible values for the attributes are:

 * ATTR_W = 1 << 0: width
 * ATTR_H = 1 << 1: height
 * ATTR_X = 1 << 2: X offset
 * ATTR_Y = 1 << 3: Y offset
 * ATTR_B = 1 << 4: border width
 * ATTR_M = 1 << 5: map state
 * ATTR_I = 1 << 6: ignore state (override_redirect)
static SCM
wm_get_attribute(xcb_window_t w, int attr)
{
	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, w);
	r = xcb_get_geometry_reply(conn, c, NULL);

	if (r == NULL)
		return -1;

	switch (attr) {
	case ATTR_X:
		attr = r->x;
		break;
	case ATTR_Y:
		attr = r->y;
		break;
	case ATTR_W:
		attr = r->width;
		break;
	case ATTR_H:
		attr = r->height;
		break;
	case ATTR_B:
		attr = r->border_width;
		break;
	case ATTR_M:
		attr = r->map_state;
		break;
	case ATTR_I:
		return scm_from_bool(r->override_redirect);
		break;
}
	free(r);
	return scm_from_bool(1); attribute not found
}
*/

/* Get the cursor position, and return its coordinates as (x y) */
static SCM
wm_get_cursor()
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, scrn->root);
	r = xcb_query_pointer_reply(conn, c, NULL);

	if (r == NULL)
		return scm_from_bool(0);

	/*

	if (r->child != XCB_NONE) {
		*x = r->win_x;
		*y = r->win_y;
	} else {
		*x = r->root_x;
		*y = r->root_y;
	}
*/
	free(r);
	return scm_from_bool(1);
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

/*
 * Change the cursor position, either relatively or absolutely, eg:
 * wm_set_cursor(-10, 20, RELATIVE);
 */
static SCM
wm_set_cursor(SCM s_x, SCM s_y, SCM s_mode)
{
	int x = scm_to_int(s_x);
	int y = scm_to_int(s_y);
	int mode = scm_to_int(s_mode);
	xcb_warp_pointer(conn, XCB_NONE, mode ? XCB_NONE : scrn->root,
			0, 0, 0, 0, x, y);
	return scm_from_bool(0);
}

/*
 * Returns 1 if a window matches the mask, 0 otherwise.
 * Possible value for the masks are:
 * LIST_HIDDEN LIST_IGNORE LIST_ALL

static SCM
wm_listable_p(SCM s_wid, SCM s_mask)
{
	enum {
		LIST_HIDDEN = 1 << 0,
		LIST_IGNORE = 1 << 1,
		LIST_ALL    = 1 << 2,
	};

	xcb_window_t wid = scm_to_uint32(s_wid);
	uint32_t mask = scm_to_uint32(s_mask);

	if ((mask & LIST_ALL)
		|| (!wm_is_mapped (wid) && mask & LIST_HIDDEN)
		|| ( wm_is_ignored(wid) && mask & LIST_IGNORE)
		|| ( wm_is_mapped (wid) && !wm_is_ignored(wid) && mask == 0))
		return scm_from_bool(1);
	return scm_from_bool(0);
}
 */

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


/*
 * Change the mapping state of a window.
 * The `mode` attribute can be one of the following: '(map unmap toggle)
static SCM
wm_remap(SCM s_wid, SCM s_mode)
{
 	xcb_window_t wid = scm_to_uint32(s_wid);
	int mode = scm_to_int(s_mode);

	enum {
		MAP    = 1 << 0,
		UNMAP  = 1 << 1,
		TOGGLE = 1 << 2
	};

	switch (mode) {
	case MAP:
		xcb_map_window(conn, wid);
		break;
	case UNMAP:
		xcb_unmap_window(conn, wid);
		break;
	case TOGGLE:
		if (wm_is_mapped(wid))
			xcb_unmap_window(conn, wid);
		else
			xcb_map_window(conn, wid);
		break;
	}
	xcb_flush(conn);
	return scm_from_bool(1);
}
 */


/* Give the input focus to the specified window */
static SCM
wm_set_focused_window(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	xcb_flush(conn);
	return scm_from_bool(1);
}

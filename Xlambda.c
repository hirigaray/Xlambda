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
static SCM window_get_list();
static SCM window_exists_p(SCM wid);
static SCM window_get_title(SCM s_wid);

static SCM window_get_border_width(SCM s_wid);
static SCM window_set_border(SCM s_wid, SCM s_width, SCM s_color);

static SCM window_hidden_p(SCM s_wid);
static SCM window_hide(SCM s_wid);
static SCM window_show(SCM s_wid);

static SCM window_ignored_p(SCM s_wid);
static SCM window_ignore(SCM s_wid);
static SCM window_manage(SCM s_wid);

static SCM window_get_focused_id();
static SCM window_set_focused_id(SCM s_wid);

static SCM window_get_geometry(SCM s_wid);
static SCM window_set_geometry(SCM s_wid, SCM s_x, SCM s_y, SCM s_width, SCM s_height);

static SCM cursor_get_position();
static SCM cursor_set_position(SCM s_x, SCM s_y);

static void*
register_functions(void* data)
{
/* scheme-name
 * args: required, optional, rest-list,
 * c_name */

	/* Window list info */
	scm_c_define_gsubr("window/list",           0, 0, 0, &window_get_list);
	scm_c_define_gsubr("window/exists?",        1, 0, 0, &window_exists_p);
	scm_c_define_gsubr("window/title?",         1, 0, 0, &window_get_title);

	/* Window attribute getters and setters. */
	scm_c_define_gsubr("window/border-width?",  1, 0, 0, &window_get_border_width);
	scm_c_define_gsubr("window/set-border!",    2, 1, 0, &window_set_border);

	scm_c_define_gsubr("window/hidden?",        1, 0, 0, &window_hidden_p);
	scm_c_define_gsubr("window/hide!",          1, 0, 0, &window_hide);
	scm_c_define_gsubr("window/show!",          1, 0, 0, &window_show);

	scm_c_define_gsubr("window/ignored?",       1, 0, 0, &window_ignored_p);
	scm_c_define_gsubr("window/ignore!",        1, 0, 0, &window_ignore);
	scm_c_define_gsubr("window/manage!",        1, 0, 0, &window_manage);

	scm_c_define_gsubr("window/current-id?",    0, 0, 0, &window_get_focused_id);
	scm_c_define_gsubr("window/focus!",         1, 0, 0, &window_set_focused_id);

	scm_c_define_gsubr("window/geometry?",      1, 0, 0, &window_get_geometry);
	scm_c_define_gsubr("window/teleport!",      5, 0, 0, &window_set_geometry);

	/* Operations on the mouse cursor. */
	scm_c_define_gsubr("cursor/position?",      0, 0, 0, &cursor_get_position);
	scm_c_define_gsubr("cursor/position!",      2, 0, 0, &cursor_set_position);

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
 * Returns the list of all existing windows.
 * First, you have the root window ID, and then they're ordered from lower to upper window.
 */
static SCM
window_get_list()
{
	xcb_window_t x_window;

	SCM s_root_wid = scm_from_uint32(scrn->root);

	/* Initialize the window list with the root window ID */
	SCM s_window_list = scm_list_1(s_root_wid);

	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(conn, scrn->root);
	r = xcb_query_tree_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_UNDEFINED; /* This is maybe unreached.
                               * the connection would have failed
                               * before this happens. */


	xcb_window_t *x_window_list = xcb_query_tree_children(r);

	/* wi = window index */
	/* children_len - 1 is due to the fact that the wids are in stacking order,
	 * and the last window is supposedly the root window, but that is not the actual case */
	for (int wi = 0; wi <= r->children_len - 1; wi++) {
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
window_exists_p(SCM wid)
{
	xcb_window_t w = scm_to_uint32(wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, w);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return SCM_BOOL_F;
	free(r);

	return SCM_BOOL_T;
}

/* Return window's border width */
static SCM
window_get_border_width(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_BOOL_F;

	SCM b = scm_from_int(r->border_width);
	free(r);

	return b;
}

/* Get a window's title */
static SCM
window_get_title(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *r;

	cookie = xcb_get_property(conn, 0, wid,
			XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0L, 32L);
	r = xcb_get_property_reply(conn, cookie, NULL);


	int title_len = 0;
	SCM t = scm_list_2(s_wid, SCM_UNDEFINED);

	if (r) {
		title_len = xcb_get_property_value_length(r);
		if (!title_len)
			return scm_list_2(s_wid, SCM_BOOL_F);

		char *title = xcb_get_property_value(r);
		free(r);

		SCM s_title = scm_from_utf8_string(title);
		t = scm_list_2(s_wid, s_title);
	}

	return t;
}

/* Return whether a window is hidden or not. */
static SCM
window_hidden_p(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_UNDEFINED;

	SCM m = SCM_UNDEFINED;
	switch (r->map_state) {
	case 0: /* Unmapped */
		m = SCM_BOOL_T;
		break;
	case 1: /* Unviewable */
		m = SCM_BOOL_T;
		break;
	case 2: /* Viewable */
		m = SCM_BOOL_F;
		break;
	default: /* Fallthrough */
		m = SCM_UNDEFINED;
	}
	free(r);

	return m;
}

/* Hide a window */
static SCM
window_hide(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_unmap_window(conn, wid);

	xcb_flush(conn);
	return scm_list_2(s_wid, SCM_BOOL_F);
}

/* Show a window */
static SCM
window_show(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_map_window(conn, wid);

	xcb_flush(conn);
	return scm_list_2(s_wid, SCM_BOOL_T);
}

/* Get the cursor position, and return its coordinates as (x y) */
static SCM
cursor_get_position()
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, scrn->root);
	r = xcb_query_pointer_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_BOOL_F;

	SCM xypos;
	if (r->child != XCB_NONE) {
		xypos = scm_list_2(scm_from_uint16(r->win_x), scm_from_uint16(r->win_y));
	} else {
		xypos = scm_list_2(scm_from_uint16(r->root_x), scm_from_uint16(r->root_y));
	}

	free(r);
	return scm_values(xypos);
}

/* Set the cursor position. */
static SCM
cursor_set_position(SCM s_x, SCM s_y)
{
	int x = scm_to_int(s_x);
	int y = scm_to_int(s_y);

	xcb_warp_pointer(conn, XCB_NONE, scrn->root, 0, 0, 0, 0, x, y);
	xcb_flush(conn);

	return scm_values(scm_list_2(s_x, s_y));
}

/* Set a window's border.
 * The color should be a hexadecimal number, eg: "#xffffff" */
static SCM
window_set_border(SCM s_wid, SCM s_width, SCM s_color)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	uint32_t width = scm_to_uint32(s_width);
	uint32_t color;

	uint32_t values[1];
  int mask = 0;
	int retval = 1;

	/* Set window border width */
	values[0] = width;
	mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
	xcb_configure_window(conn, wid, mask, values);

	/* Optionally set color of window border */
	if (s_color != SCM_UNDEFINED) {
		color = scm_to_uint32(s_color);
		values[0] = color;
		mask = XCB_CW_BORDER_PIXEL;
		xcb_change_window_attributes(conn, wid, mask, values);
		retval++;
	}

	xcb_flush(conn);
	return scm_from_int(retval);
}

/* Return whether a given window should be ignored by Xlambda or not */
static SCM
window_ignored_p(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_UNDEFINED; /* this is probably the root window
                               * in any case, it shouldn't be handled. */

    /* override_redirect is 0 when the window shouldn't be handled by Xlambda
     * and it's 1 otherwise. scm_from_bool returns #f on the first case, and #t
     * for the latter, so it's fine to just that here. */
	SCM i = scm_from_bool(r->override_redirect);

	free(r);
	return i;
}

/* Unignore a window */
static SCM
window_manage(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { 0 };

	xcb_change_window_attributes(conn, wid, mask, val);

	return scm_list_2(s_wid, SCM_BOOL_F);
}

/* Ignore a window */
static SCM
window_ignore(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { 1 };

	xcb_change_window_attributes(conn, wid, mask, val);

	return scm_list_2(s_wid, SCM_BOOL_T);
}

/* Get focused window identifier */
static SCM
window_get_focused_id()
{
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_BOOL_F;

	SCM f_wid = scm_from_uint32(r->focus);
	free(r);

	return f_wid;
}

/* Give the input focus to the specified window */
static SCM
window_set_focused_id(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	/* Bring window to the front */
	xcb_configure_window(conn, wid, XCB_STACK_MODE_ABOVE, 0);
	xcb_flush(conn);
	return scm_list_2(s_wid, SCM_BOOL_T);
}

/* Return window's geometry. */
static SCM
window_get_geometry(SCM s_wid)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	xcb_get_geometry_cookie_t c;
	xcb_get_geometry_reply_t *r;

	c = xcb_get_geometry(conn, wid);
	r = xcb_get_geometry_reply(conn, c, NULL);
	if (r == NULL)
		return SCM_BOOL_F;

	SCM x = scm_from_int16(r->x);
	SCM y = scm_from_int16(r->y);
	SCM w = scm_from_uint16(r->width);
	SCM h = scm_from_uint16(r->height);
	free(r);

	return scm_values(scm_list_4(x, y, w, h));
}

/* Set a window's geometry. */
static SCM
window_set_geometry(SCM s_wid, SCM s_x, SCM s_y, SCM s_width, SCM s_height)
{
	xcb_window_t wid = scm_to_uint32(s_wid);

	uint32_t mask =   XCB_CONFIG_WINDOW_X
	                | XCB_CONFIG_WINDOW_Y
	                | XCB_CONFIG_WINDOW_WIDTH
	                | XCB_CONFIG_WINDOW_HEIGHT;

	uint32_t values[4];
	values[0] = scm_to_uint32(s_x);
	values[1] = scm_to_uint32(s_y);
	values[2] = scm_to_uint32(s_width);
	values[3]= scm_to_uint32(s_height);

	xcb_configure_window(conn, wid, mask, values);

	xcb_flush(conn);
	return scm_values(scm_list_4(s_x, s_y, s_width, s_height));
}

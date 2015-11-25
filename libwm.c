#include <xcb/xcb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wm.h"

int
wm_init_xcb()
{
	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn))
		return 0;
	return 1;
}

int
wm_kill_xcb()
{
	if (conn) {
		xcb_disconnect(conn);
		return 1;
	}
	return 0;
}

int
wm_is_alive(xcb_window_t w)
{
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, w);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	free(r);
	return 1;
}

int
wm_is_mapped(xcb_window_t w)
{
	int ms;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, w);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	ms = r->map_state;

	free(r);
	return ms == XCB_MAP_STATE_VIEWABLE;
}

int
wm_is_ignored(xcb_window_t wid)
{
	int or;
	xcb_get_window_attributes_cookie_t c;
	xcb_get_window_attributes_reply_t  *r;

	c = xcb_get_window_attributes(conn, wid);
	r = xcb_get_window_attributes_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	or = r->override_redirect;

	free(r);
	return or;
}

int
wm_get_screen()
{
	scrn = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if (scrn == NULL)
		return 0;
	return 1;
}

int
wm_get_windows(xcb_window_t w, xcb_window_t **l)
{
	uint32_t childnum = 0;
	xcb_query_tree_cookie_t c;
	xcb_query_tree_reply_t *r;

	c = xcb_query_tree(conn, w);
	r = xcb_query_tree_reply(conn, c, NULL);
	if (r == NULL)
		return -1;

	*l = malloc(sizeof(xcb_window_t) * r->children_len);
	memcpy(*l, xcb_query_tree_children(r),
			sizeof(xcb_window_t) * r->children_len);

	childnum = r->children_len;

	free(r);
	return childnum;
}

xcb_window_t
wm_get_focus(void)
{
	xcb_window_t w = 0;
	xcb_get_input_focus_cookie_t c;
	xcb_get_input_focus_reply_t *r;

	c = xcb_get_input_focus(conn);
	r = xcb_get_input_focus_reply(conn, c, NULL);
	if (r == NULL)
		return -1;

	w = r->focus;
	free(r);
	return w;
}


int
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
	}

	free(r);
	return attr;
}

int
wm_get_cursor(int mode, uint32_t wid, int *x, int *y)
{
	xcb_query_pointer_reply_t *r;
	xcb_query_pointer_cookie_t c;

	c = xcb_query_pointer(conn, wid);
	r = xcb_query_pointer_reply(conn, c, NULL);

	if (r == NULL)
		return 0;

	if (r->child != XCB_NONE) {
		*x = r->win_x;
		*y = r->win_y;
	} else {
		*x = r->root_x;
		*y = r->root_y;
	}

	return 1;
}

int
wm_set_border(int width, int color, xcb_window_t win)
{
	uint32_t values[1];
	int mask, retval = 0;
	/* change width if > 0 */
	if (width > -1) {
		values[0] = width;
		mask = XCB_CONFIG_WINDOW_BORDER_WIDTH;
		xcb_configure_window(conn, win, mask, values);
		retval++;
	}

	/* change color if > 0 */
	if (color > -1) {
		values[0] = color;
		mask = XCB_CW_BORDER_PIXEL;
		xcb_change_window_attributes(conn, win, mask, values);
		retval++;
	}

	xcb_flush(conn);
	return retval;
}

int
wm_set_cursor(int x, int y, int mode)
{
	xcb_warp_pointer(conn, XCB_NONE, mode ? XCB_NONE : scrn->root,
			0, 0, 0, 0, x, y);
	return 1;
}

int
wm_is_listable(xcb_window_t w, int mask)
{
	if ((mask & LIST_ALL)
		|| (!wm_is_mapped (w) && mask & LIST_HIDDEN)
		|| ( wm_is_ignored(w) && mask & LIST_IGNORE)
		|| ( wm_is_mapped (w) && !wm_is_ignored(w) && mask == 0))
		return 1;

	return 0;
}

int
wm_teleport(xcb_window_t wid, int x, int y, int w, int h)
{
	uint32_t values[4];
	uint32_t mask =   XCB_CONFIG_WINDOW_X
	                | XCB_CONFIG_WINDOW_Y
	                | XCB_CONFIG_WINDOW_WIDTH
	                | XCB_CONFIG_WINDOW_HEIGHT;
	values[0] = x;
	values[1] = y;
	values[2] = w;
	values[3] = h;
	xcb_configure_window(conn, wid, mask, values);

	xcb_flush(conn);
	return 1;
}

int
wm_move(xcb_window_t wid, int mode, int x, int y)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;
	
	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		x += curx;
		y += cury;
	}

	/* the following prevent windows from moving off the screen */
	if (x < 0)
		x = 0;
	else if (x > scrn->width_in_pixels - curw - 2*curb)
		x = scrn->width_in_pixels - curw - 2*curb;

	if (y < 0)
		y = 0;
	else if (y > scrn->height_in_pixels - curh - 2*curb)
		y = scrn->height_in_pixels - curh - 2*curb;

	wm_teleport(wid, x, y, curw, curh);
	return 1;
}

void
wm_set_override(xcb_window_t w, int or)
{
	uint32_t mask = XCB_CW_OVERRIDE_REDIRECT;
	uint32_t val[] = { or };

	xcb_change_window_attributes(conn, w, mask, val);
}


int
wm_remap(xcb_window_t wid, int mode)
{
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
	return 1;
}

int
wm_resize(xcb_window_t wid, int mode, int w, int h)
{
	int curx, cury, curw, curh, curb;

	if (!wm_is_mapped(wid) || wid == scrn->root)
		return -1;
	
	curb = wm_get_attribute(wid, ATTR_B);
	curx = wm_get_attribute(wid, ATTR_X);
	cury = wm_get_attribute(wid, ATTR_Y);
	curw = wm_get_attribute(wid, ATTR_W);
	curh = wm_get_attribute(wid, ATTR_H);

	if (mode == RELATIVE) {
		w += curw;
		h += curh;
	} else {
		w -= curx;
		h -= cury;
	}

	/*
	 * The following prevent windows from growing out of the screen, or
	 * having a negative size
	 */
	if (w < 0)
		w = curw;
	if (curx + w >  scrn->width_in_pixels)
		w = scrn->width_in_pixels - curx - 2*curb;

	if (h < 0)
		h = curh;
	if (cury + h > scrn->height_in_pixels)
		h = scrn->height_in_pixels - cury - 2*curb;

	wm_teleport(wid, curx, cury, w, h);
	return 1;
}

int
wm_restack(xcb_window_t wid, uint32_t mode)
{
	uint32_t values[1] = { mode };
	xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, values);
	xcb_flush(conn);
	return 1;
}

int
wm_set_focus(xcb_window_t wid)
{
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid,
	                    XCB_CURRENT_TIME);
	xcb_flush(conn);
	return 1;
}

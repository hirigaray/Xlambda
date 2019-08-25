# Xlambda

A window manager for X that is extensible in Guile Scheme

# How to use this
Xlambda currently ships with the following basic functions:

	`(window/list)`

	List all open windows, including hidden windows and unmapped windows.

	`(window/exists? wid)`

	Check if a window exists.

	`(window/title? wid)`

	Return the title of a window.

	`(window/border-width? wid)`

	Return the border width of a window.

	`(window/hidden? wid)`

	Check if a window is hidden.

	`(window/hide! wid)`

	Hide a window.

	`(window/show! wid)`

	Show a window.

	`(window/ignored? wid)`

	Check if a window is ignored by the window manager.

	`(window/ignore! wid)`

	Ignore a window.

	`(window/manage! wid)`

	Unignore a window.

	`(window/current-id?)`

	Return the currently focused window ID.

	`(window/focus! wid)`

	Focus a window.

	`(window/geometry? wid)`

	Get the geometry of a window. This returns 4 values, x position, y position, width, height.

	`(window/teleport! wid x y w h)`

	Teleport a window to the given coordinates.

	`(cursor/position?)`

	Return the current position of the mouse cursor. This returns 2 values, x position, y position.

	`(cursor/position!)`

	Set the position of the mouse cursor.

As there is currently no mouse support, Xlambda must be used with another
window manager in order to be useful. However, you can already create all sorts of window
management scripts with Xlambda! It already ships with a few.
Load them with `Xlambda -l examples/definitions.scm`

See the following sections for more details of future plans.

## Why?
This is serving as an experiment in learning what the fundamental operations of window management
are. So far, as I understand this, `(wm/teleport!)` seems to be the most powerful operation,
because all other functions related to window management can be implenmented on top of it.

All the other functions that have shipped in V1 are functions mostly required to get the
information needed to make this an useful tool, and less of a thought experiment.

On the other hand, if Xlambda is looking to be a practical window manager, the "most abstract"
version of it is something I've been thinking about for some time now, which I've jokingly called
the "Window Manager Calculus". This would be the formal system that would be able to describe
all the operations needed to implement any possible form of window management conceivable.

Considering that window managenment is mostly Geometry and Algebra, I don't think it is that
unique of an idea, but I haven't been able to find work on this, in these terms.

## Currently planned roadmap
Xlambda uses reverse Kelvin versioning, that is to say, the bigger the version number is,
the less "frozen" it is, and the more versatile it becomes.
* V1 `(current version)`
Ship with basic functionality, but enough to program whatever window layouts are imaginable,
due to `(wm/teleport!)`
* V2 `(work in progress)`
Implement event watching support (and mouse support)
This took a long time in between versions because I was trying to rewrite Xlambda in pure guile, using guile-xcb.

That won't work for now. That's fine.

I'm gonna use libuv for the async aspects, for now.

By this point, Xlambda should be easily usable as a standalone window manager.
* V3
Ship with more example functions, namely, various examples of tiling functions.
* V4
Start using a remote REPL, so Xlambda would be a daemon, controllable by any Guile instance.
* ...
* V273
Hopefully, by this point, we'll have subsumed every single window manager in existence,
due to the formalization of the Window Management Calculus(tm)

## Support this project!
If you like these ideas, please consider donating to my Patreon! It would be *immensely* helpful
towards me continuing to work on these projects.

Here's the link: https://patreon.com/lmilon

Also, consider supporting the Guile project! See https://www.gnu.org/software/guile/contribute/
for more information.

## Thanks
* Special thanks to Danny O'Brien, for supporting me on Patreon!

* dcat and z3bra for making libwm, upon which Xlambda was based
* the Guile developers, for making this FFI so easy to use!
* You, for trying this out!

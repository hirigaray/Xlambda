# Xlambda

A window manager for X that is extensible in Guile Scheme

# How to use this
Xlambda currently ships with the following basic functions:

`  (wm/window/x)`
Returns the x coordinate of the top-leftmost pixel of a window (including borders).

`  (wm/window/y)`
Returns the y coordinate of the top-leftmost pixel of a window (including borders).

`  (wm/window/w) wid`
Returns a window's width, in pixels.

`  (wm/window/h wid)`
Returns a window's height, in pixels.

`  (wm/window/b wid)`
Returns a window's border's width, in pixels.

`  (wm/window/m wid)`
Returns #t, if a window is mapped by the window manager, or #f otherwise.

`  (wm/window/i wid)`
Returns #t, if a window is ignored by the window manager, or #f otherwise.

`  (wm/window/map wid)`
Map a window.

`  (wm/window/unmap wid)`
Unmap a window.

`  (wm/get-focused)`
Get the currently focused window's ID.

`  (wm/set-focused wid)`    
Get the currently focused window's ID.

`  (wm/get-cursor-pos)`
Get the current position for the mouse cursor, as a list with the coordinates as (x y).

`  (wm/set-cursor-pos! x y)`
Set the position for the mouse cursor.

`  (wm/list-windows)`
List all windows open, including hidden ones (this behavior will be changed).

`  (wm/exists? wid)`        
Return #t if the given window exists, #f otherwise.

`  (wm/teleport! wid x y w h)`
Teleport a window to the x and y coordinates, with a width w, and a height h.

As there is currently no mouse support, Xlambda must be used with another
window manager in order to be useful. However, you can already create all sorts of window
management scripts with Xlambda! It already ships with a few. Load them with `Xlambda -l examples/definitions.scm`

See the following sections for more details of future plans.

## Help needed!
I wrote Xlambda under OpenBSD, and the Makefile should work as long as you have guile2.2 installed.
I haven't tested this under Linux yet, so any help in making the Makefile more portable is greatly appreciated!

## Why?
This is serving as an experiment in learning what the fundamental operations of window management
are. As far as I understand so far, `(wm/teleport!)`` seems to be the most powerful operation,
because all other functions related to window management can be implenmented on top of it.

All other functions that have existed in version 0.1 are functions mostly required to get the
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
Ship with basic functionality, but enough to script whatever window layouts are imaginable,
due to `(wm/teleport!)`
* V2
Ship with more example functions, namely, various examples of tiling functions.
* V3
Implement event watching support (and mouse support)
By this point, Xlambda should be easily usable as a standalone window manager.
* V4
Start using a remote REPL, so Xlambda would be a daemon, controllable by any Guile instance.
* ...
* V273
Hopefully, by this point, we'll have had subsumed every single window manager in existence,
due to the formalization of the Window Management Calculus(tm)

## Support this project!
If you like these ideas, please consider donating to my Patreon! It would be *immensely* helpful
towards me continuing to work on these projects.

Here's the link: https://patreon.com/lmilon

Also, consider supporting the Guile project! See https://www.gnu.org/software/guile/contribute/ 
for more information.

## Thanks
* dcat and z3bra for making libwm, upon which Xlambda was based
* the Guile developers, for making this FFI so easy to use!
* You, for trying this out!

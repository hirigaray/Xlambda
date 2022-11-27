# Xlambda

A window manager for X that is extensible in Scheme

## Why?
This is serving as an experiment in learning what the fundamental operations of window management
are. So far, as I understand this, `(wm/teleport!)` seems to be the most powerful operation,
because all other functions related to window management can be implemented on top of it.

All the other functions that have shipped in V1 are functions mostly required to get the
information needed to make this an useful tool, and less of a thought experiment.

On the other hand, if Xlambda is looking to be a practical window manager (which it is), the "most abstract" version of it is something I've been thinking about for some time now, which I've jokingly called the "Window Manager Calculus". This would be the formal system that would be able to describe all the operations needed to implement any possible form of window management conceivable.

Considering that window managenment is mostly Geometry and Algebra, I don't think it is that
unique of an idea, but I haven't been able to find work on this, in these terms.

## How?
Xlambda, in this development stage, uses reverse Kelvin versioning, that is to say, the bigger the version number is,
the less "frozen" it is, and the more versatile it becomes.

With that said, once I think it's worth using for real, I will drop this pretense and move onto semver to make packaging not a total nightmare :)

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

## Thanks
* God, for creating the universe and everything that is.
* Danny O'Brien, for supporting me on Patreon, the first time Xlambda came around.
* dcat and z3bra for making libwm, upon which Xlambda was based, originally.
* You, for trying this out!

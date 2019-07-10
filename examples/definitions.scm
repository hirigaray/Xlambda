(define root-wid (car (wm/list-windows)))
(define screen-width (wm/window/w root-wid))
(define screen-height (wm/window/h root-wid))
(define border-width 1)
(define gap-size 16)

(define wm/fullscreen-current!
  (lambda ()
    (wm/teleport!
      (wm/get-focused)
      0 ; x coordinate
      0 ; y coordinate
      (- screen-width  (* 2 border-width))
      (- screen-height (* 2 border-width)))))

(define wm/corner-top-left!
  (lambda ()
    (let [(wid (wm/get-focused))]
      (wm/teleport!
        wid
        gap-size
        gap-size
        (wm/window/w wid)
        (wm/window/h wid)))))

(define wm/corner-top-right!
  (lambda ()
    (let* [(wid (wm/get-focused))
           (window-width  (wm/window/w wid))
           (window-height (wm/window/h wid))]
      (wm/teleport!
        wid
        (- screen-width window-width gap-size)
        gap-size
        window-width
        window-height))))

; Even further abstractions can be built, these are just simple
; examples of how powerful and legible window management with Scheme can be.

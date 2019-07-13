; let-values and let*-values
(use-modules (srfi srfi-11))

(define root-wid (car (window/list)))
(define-values
  (screen-x ; these don't make much sense, but as far as I know
   screen-y ; you can't go without binding values
   screen-width
   screen-height)
  (window/geometry? root-wid))

(define border-width 1)
(define gap-size     16)

(define (window/fullscreen! wid)
  (window/teleport! wid
    0 ; x coordinate
    0 ; y coordinate
    (- screen-width  (* 2 border-width))
    (- screen-height (* 2 border-width))))

(define (window/corner-top-left! wid)
  (let*-values ([cur (window/current-id?)]
                [(x y w h) (window/geometry? wid)])
    (window/teleport! cur
                      gap-size gap-size w h)))

(define (window/corner-top-right! wid)
  (let*-values ([(x y w h) (window/geometry? wid)])
    (window/teleport! wid
      (- screen-width w gap-size) gap-size w h)))

(define (window/double-gap-top-left! wid)
  (let-values ([(x y w h) (window/geometry? wid)])
    (window/teleport! wid
      (* 2 x) (* 2 y) w h)))

; Even further abstractions can be built, these are just simple
; examples of how powerful and legible window management with Scheme can be.

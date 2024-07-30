(define x 3)
(set! x 5)
(let ((x 4) (y 7))
  (begin
   (set! x 12)
   (set! y 15)
   (set! y (+ x 7))
    y))

(
(let ((x 3) (y 5) (z 6) (w 7))
  (cond ((= x y) (lambda (x) (+ x 1)))
        ((= x x) (lambda (x) (+ x 2)))
        (#t 12)))

14
)

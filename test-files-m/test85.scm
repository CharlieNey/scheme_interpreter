(let ((x 1) (y 2) (z 3))
  (begin
    (let ((y 4))
      (begin
        (set! y 5)
        (set! x (+ y 1)))
      (+ x y z))))

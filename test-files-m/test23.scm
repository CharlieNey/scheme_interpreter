(let ((x 3) (y 5))
  (begin
    x
    y
    (set! y 7)
    (let ((z y))
      y)))

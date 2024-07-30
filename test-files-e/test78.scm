(define y 5)
  (letrec ((x y) (y x))
      x)
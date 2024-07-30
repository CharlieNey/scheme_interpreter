(letrec ((sum (lambda (x)
                (if (= x 0)
                    0
                    (+ x (sum (- x 1)))))))
  (sum 5))

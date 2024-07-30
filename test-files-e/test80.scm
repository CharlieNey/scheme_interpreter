(let* ((f (lambda (x) (if (= 0 x) 0 (f (- x 1))))))
  (f 3))
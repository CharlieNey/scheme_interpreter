;; Knuth test.
(define less-than-or-equal
  (lambda (x y)
    (if (> x y) #f #t)))

(define a
  (lambda (k x1 x2 x3 x4 x5)
    (letrec ((b
              (lambda ()
                (begin
                  (set! k (- k 1))
                  (a k b x1 x2 x3 x4)))))
      (if (less-than-or-equal k 0)
          (+ (x4) (x5))
          (b)))))

(a 10 (lambda () 1) (lambda () -1)
   (lambda () -1) (lambda () 1)
   (lambda () 0))

(define make-account
  (let ((max-withdrawal 10)) ; let goes here for shared variables   
    (lambda (balance)
       (lambda (amt)
         (begin (if (< amt max-withdrawal)
           (set! balance (- balance amt))
           (set! max-withdrawal (+ max-withdrawal 1)))
         balance)))))
         
(define mine (make-account 30))
(define yours (make-account 60))
(mine 4)
(mine 40)
(mine 10)
(yours 10)
(yours 50)
(yours 6)
(mine 3)

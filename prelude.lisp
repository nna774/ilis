(define cadr (lambda (x) (car (cdr x))))
(define cddr (lambda (x) (cdr (cdr x))))

(define null (lambda (x) (if (eq '() '()) #t #f)))
(define list (lambda xs xs))

(define add (lambda (x y)
  (if (eq y 0)
    x
    (if (eq (sign y) 1)
      (add (inc x) (dec y))
      (add (dec x) (inc y))))))

(define neg (lambda (x)
  (if (eq x 0)
    0
    (if (eq (sign x) 1)
      (dec (neg (dec x)))
      (inc (neg (inc x)))))))

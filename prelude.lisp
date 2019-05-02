(define cadr (lambda (x) (car (cdr x))))
(define cddr (lambda (x) (cdr (cdr x))))

(define null (lambda (x) (if (eq '() '()) #t #f)))
(define list (lambda xs xs))

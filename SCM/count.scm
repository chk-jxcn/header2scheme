(define (count the-list)
  (define (count-recur input)
    (if (pair? input)
	(if (list? input)
	    (+ (count-recur (car input))
	       (count-recur (cdr input)))
	    2)
	(if (null? input)
	    0
	    1)))
  (if (list? the-list)
      (count-recur the-list)
      'error))

	       

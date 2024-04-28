;; Scoping for loads -- UNIX only. kbrussel@media.mit.edu 1/97
(define *file-load-scope* '())
(define (absolute-path? path)
  (if (not (string? path))
      #f
      (if (< (string-length path) 1)
	  #f
	  (char=? #\/ (string-ref path 0)))))
(define (file-load-vicinity)
  (define (file-load-vicinity-aux path-string file-load-scope)
    (if (null? file-load-scope)
	path-string
	(if (absolute-path? (car file-load-scope))
	    (string-append (car file-load-scope) path-string)
	    (file-load-vicinity-aux (string-append (car file-load-scope)
						   path-string)
				    (cdr file-load-scope)))))
  (file-load-vicinity-aux "" *file-load-scope*))

(define (last-slash-posn pathname)
  (define (lsp-aux pathname index)
    (if (< index 0) 
	#f
	(if (char=? #\/ (string-ref pathname index))
	    index
	    (lsp-aux pathname (- index 1)))))
  (lsp-aux pathname (- (string-length pathname) 1)))

(define (file-path pathname)
  (let ((last-index (last-slash-posn pathname)))
    (if (not last-index)
	""
	(substring pathname 0 (1+ last-index)))))
(define (file-name pathname)
  (let ((last-index (last-slash-posn pathname)))
    (if (not last-index)
	pathname
	(substring pathname (1+ last-index) (string-length pathname)))))

(define (push-file-scope pathname)
  (set! *file-load-scope* (cons (file-path pathname)
				*file-load-scope*)))
(define (pop-file-scope)
  (set! *file-load-scope* (cdr *file-load-scope*)))

;; Tests
(define *file-load-scope* '("foo/bar" "/usr/local/lib/" "/tmp"))
(file-load-vicinity)
(last-slash-posn "foo.scm")
(file-path "foo.scm")
(push-file-scope "/usr/local/lib/")
(pop-file-scope)
*file-load-scope*

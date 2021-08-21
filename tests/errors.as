;longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong comment

.asciz " a very long and even valid data line that can be read anyway, trying to find the words to fill this line"

.extern	ExternLabel
bne     $23, $23, ExternLabel
call	NotDefined
.entry	ExternLabel
.entry NotExistsEntry

;add     $0 ,$1 ,$2 , 
sub     $3 ,,$4 ,$5
and     ,$6 ,$7 ,$8
or      $9 ,$1,0,$11
nor     $12,$13, $45

label?: add 0, 1, 2

Add $0, $31, $15
.dw     0,5,-3,+80, 1,1 ,-2147483649,2147483647
.dw     0,5,-3,+80, 1,1 ,-2147483648,2147483648

.asciz	"Bj��rk����oacute�"

thislabelisvalidbuttoolongwecanreaditanyway: add $0 ,$1, 2

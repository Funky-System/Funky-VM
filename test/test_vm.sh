#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

run_test_expect() {
    echo -e "locals.res 0\n\n" > .tmp_test.fasm
    cat /dev/stdin >> .tmp_test.fasm
    echo -e "st.reg %r0\nlocals.cleanup\nld.reg %r0\ntrap 2\npop\n" >> .tmp_test.fasm
    printf "%-50s" "$1"
    ${DIR}/funky-as .tmp_test.fasm -o .tmp_test.funk
    output=$(${DIR}/funky-vm .tmp_test)
    if [ "$2" == "$output" ]; then
        echo -e "[  \033[32mOK\033[0m  ] ${output//$'\n'/\\\\n}"
    else
        echo -e "[ \033[31mFAIL\033[0m ] ${output//$'\n'/\\\\n}"
    fi

    rm .tmp_test.fasm .tmp_test.funk
}

run_test_expect "simple ldc 1" 4 << EOF
    ld.int 4
EOF

run_test_expect "simple ldc 2" 5 << EOF
    ld.int 4
    ld.int 5
EOF

run_test_expect "simple ldc 3" 32234 << EOF
    ld.uint 32234
EOF

run_test_expect "simple lds 1" 16 << EOF
    ld.int 15
    ld.uint 16
    ld.int 17
    ld.stack -1
EOF

run_test_expect "Arithmetics (add)" 8 << EOF
    ld.int 3
    ld.int 5
    add
EOF

run_test_expect "Arithmetics (sub)" 8 << EOF
    ld.int 10
    ld.int 2
    sub
EOF

run_test_expect "Arithmetics (mul)" 15 << EOF
    ld.uint 3
    ld.uint 5
    mul
EOF

run_test_expect "Arithmetics (float mul)" 168.000000 << EOF
    ld.uint 30
    ld.float 5.6
    mul
EOF

run_test_expect "Arithmetics (div)" 5 << EOF
    ld.int 10
    ld.int 2
    div
EOF

run_test_expect "Arithmetics (div)" -5 << EOF
    ld.int 10
    ld.int -2
    div
EOF

run_test_expect "Arithmetics (combination)" 32 << EOF
	LD.int	4
	LD.int	5
	MUL
	LD.int	2
	LD.int	6
	MUL
	ADD
EOF

run_test_expect "Bitwise shifts" 4 << EOF
ld.int 1
ld.int 1
lsh
ld.int 2
lsh
ld.int 1
rsh
EOF

run_test_expect "Swap (swp)" 10 << EOF
    ld.int 10
    ld.int 5
    swp
EOF

run_test_expect "Simple subroutine and loop" 720 << EOF
	jmp	main
Fac:	locals.res	1	# fac( int n )
	LD.int	    1	# int res = 1 ;
	st.local	0
FacTst:	ld.local	-4	# while ( n > 1 )
	LD.int	1
	gt
	brfalse	FacEnd
	ld.local	-4	# res = res * n
	ld.local	0
	mul
	st.local	0
	ld.local	-4	# n = n - 1
	LD.int	1
	sub
	st.local	-4
	jmp	FacTst
FacEnd:	ld.local	0	# return( res )
	st.reg	%RR
	locals.cleanup
	ret
main:	LD.int	6
	call	Fac, 0	# fac( 6 )
	ld.reg %rr
EOF

run_test_expect "Simple recursion" 5040 << EOF
	jmp	main
Fac:	locals.res	0	# fac( int n )
	ld.local	-4	# if ( n <= 1 )
	LD.int	1
	le
	brtrue	FacTh
	ld.local	-4	# else return( n * fac( n-1 ) ) ;
	ld.local	-4
	LD.int	1
	sub
	call	Fac, 0
	pop
	ld.reg	%RR
	mul
	jmp	FacEnd
FacTh:	LD.int	1	# then return( 1 )
FacEnd:	st.reg	%RR
	locals.cleanup
	ret
main:	LD.int	7
	call	Fac, 0	# fac( 7 )
	ld.reg %rr
EOF

run_test_expect "If branch" 1 << EOF
	jmp	Main
DoIf:	locals.res	1
	LD.local	-4
	LD.int	2
	MOD
	LD.int	0
	ne
	brtrue	Then
	jmp	IfEnd
Then:
    ld.int 1
	st.reg %RR
IfEnd:	locals.cleanup
	RET
Main:
    LD.int	11
	call	DoIf, 0
    ld.reg %rr
EOF

run_test_expect "Else branch" 0 << EOF
	jmp	Main
DoIf:	locals.res	1
	LD.local	-4
	LD.int	2
	MOD
	LD.int	0
	ne
	brtrue	Then
	jmp	IfEnd
Then:
    ld.int 1
	st.reg %RR
IfEnd:	locals.cleanup
	RET
Main:
    LD.int	10
	call	DoIf, 0
    ld.reg %rr
EOF

run_test_expect "Printing text" $"Hello!
1" << EOF
ld.int 'H'
ld.int 'e'
ld.int 'l'
ld.int 'l'
ld.int 'o'
ld.int '!'
ld.int '\n'
ld.int '\0'
ld.sref -7
int 10
ajs -8
ld.int 1
EOF

run_test_expect "Strings" "9 y" << EOF
ld.str "I am "
ld.int 29
conv.str
str.concat
ld.str " years old."
str.concat
ld.int 6
ld.int 3
str.substr
EOF

run_test_expect "String add operator" "abcdef" << EOF
ld.str "abc"
ld.str "def"
add
EOF

run_test_expect "String add operator with integer" "abc1" << EOF
ld.str "abc"
ld.int 1
add
EOF

run_test_expect "Arrays" 22 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.local 0
ld.int 2
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (init)" 3 << EOF
ld.int 1
ld.int 2
ld.int 3
ld.arr 3
ld.int 2
ld.arrelem
EOF

run_test_expect "Arrays (init length)" 3 << EOF
ld.int 1
ld.int 2
ld.int 3
ld.arr 3
arr.len
EOF

run_test_expect "Arrays (length)" 4 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.local 0
arr.len
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (delete)" 33 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.local 0
ld.int 2
del.arrelem

ld.local 0
ld.int 2
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (insert 1)" 666 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.int 666
ld.local 0
ld.int 3
arr.insert

ld.local 0
ld.int 3
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (insert 2)" 33 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.int 666
ld.local 0
ld.int 3
arr.insert

ld.local 0
ld.int 4
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (insert 3)" 44 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.int 44
ld.local 0
ld.int 5
arr.insert

ld.local 0
ld.int 5
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (slice)" 22 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.int 44
ld.local 0
ld.int 5
arr.insert

ld.local 0
ld.int 1
ld.int 3
arr.slice
ld.int 1
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (slice)" 4 << EOF
locals.res 1
ld.arr 0
st.local 0

ld.int 11
ld.local 0
ld.int 0
st.arrelem

ld.int 22
ld.local 0
ld.int 2
st.arrelem

ld.int 33
ld.local 0
ld.int 3
st.arrelem

ld.int 44
ld.local 0
ld.int 5
arr.insert

ld.local 0
ld.int 1
ld.int -2
arr.slice
arr.len
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (concat 1)" 4 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.int 3
ld.local 1
ld.int 0
st.arrelem

ld.int 4
ld.local 1
ld.int 1
st.arrelem

ld.local 0
ld.local 1
arr.concat
arr.len
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (concat 2)" 3 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.int 3
ld.local 1
ld.int 0
st.arrelem

ld.int 4
ld.local 1
ld.int 1
st.arrelem

ld.local 0
ld.local 1
arr.concat
ld.int 2
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (+ operator)" 3 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.int 3
ld.local 1
ld.int 0
st.arrelem

ld.int 4
ld.local 1
ld.int 1
st.arrelem

ld.local 0
ld.local 1
add
ld.int 2
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (+ operator, length)" 3 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.int 88
ld.local 0
add
arr.len
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (+ operator, append)" 88 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.local 0
ld.int 88
add

ld.int 2
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

run_test_expect "Arrays (copy)" 2 << EOF
locals.res 2
ld.arr 0
st.local 0
ld.arr 0
st.local 1

ld.int 1
ld.local 0
ld.int 0
st.arrelem

ld.int 2
ld.local 0
ld.int 1
st.arrelem

ld.local 0
arr.copy
st.local 1

ld.int 9
ld.local 0
ld.int 1
st.arrelem

ld.local 1
ld.int 1
ld.arrelem
st.reg %r0
locals.cleanup
ld.reg %r0
EOF

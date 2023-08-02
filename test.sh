#! /bin/sh
#
#echo "Compiling..."
#gcc -lm -fsanitize=address -Isrc -o main.out src/*.c
#gcc -Wall -Wextra -pedantic -lm -g -Isrc -o main-val.out src/*.c || exit 1

#echo "C test"
#gcc -O0 -Isrc -o test.out tests/test.c
#./test.out

./main.out '( PRINT test ) 15 h. ." vs 0xF, " 7 dup . b. ." vs 0b0111 " ;'
./main.out '( BASE test ) b111 . ." vs 7, " h99 dup . h. ." vs 0x99 " ;'

./main.out '( IF test ) 1 ? ." PASS " : ." FAIL " ; 0 ? ." FAIL " : ." PASS " ; . ;'
./main.out '( FOR test ) 5 FOR dup . ; . . ;'
./main.out '( CASE test ) 3 ?: 1 : ." FAIL " ; 2 : ." FAIL " ; 3 : ." PASS " ; 4 : ." FAIL " ;: nl . . ;'

./main.out '( SWAP test ) 1 2 swp . . ." vs 1 2 " nl ;'
./main.out '( MATH test ) 1 2 + . ." vs 3, " 5 2 - . ." vs 3, " ;'
./main.out '( * / test ) 2 3 * . ." vs 6, " 10 2 / . ." vs 5 " ;'

./main.out '( MEM test ) 3 0 set . 0 get . ." vs 3, " D0 . ;'
./main.out '( VAR test ) var test dup . . ;'

./main.out '( FLOAT test ) 314 100 f/ f. ." vs 3.14, " 3000 f> 1000 f> f/ f. ." vs 3.0 " ;'
./main.out '( FLOAT MATH test ) 1 100 f/ 1 100 f/ sf. f+ f. ." vs 0.02 " . ;'

# 0 10k | ovr -> 0 10k 0 | ovr -> 0 10k 0 10k | + -> 0 10k 10k+0 | swp -> 0 10k+0 10k | rot drp
./main.out '( LOOP test ) 0 360 FOR ovr ovr + swp rot drp ; u. ;'
./main.out '0 0 set 30000 FOR dup 1000 > ? 1 : 2 ; 0 get + 0 set ; 0 get . ;'

./main.out '( FUNC test ) : sqr dup * :; 2 sqr dup . sqr . : trg dup dup * * :; 2 trg . ;'
./main.out '( FUNC NUM test ) : add 3 + :; 2 add . ;'

./main.out '( DELAY test ) 1000 ms ;'

./main.out '( FIB test ) : fib :;'
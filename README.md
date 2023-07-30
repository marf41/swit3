# SWIT3

Simple, small, somewhat fast, FORTH inspired programming language.

Very early WIP.

- only three first characters of word are parsed (`login`, `logic`, and `log` are equal)
- stack based
- code is case insensitive

| What | Words | Example | Stack before | Stack after |
| ---- | ----- | ------- | ------------ | ----------- |
| COMMENT | `( )` | `( comment )` | `-` | `-` |
| LITERAL | `#` | `#1` | `-` | `value` |
| PRINT | `.` | `#1 .` | `value` | `-` |
| UNSIGNED PRINT | `.` | `#1 u.` | `value` | `-` |
| HEX PRINT | `.` | `#1 h.` | `value` | `-` |
| BINARY PRINT | `.` | `#1 b.` | `value` | `-` |
| FLOAT PRINT | `.` | `#1 f> f.` | `value` | `-` |
| IF | `? " ;` | `condition ? if true : if false ;` | `condition` | `-` |
| FOR | `FOR ;` | `counter FOR ... ;` | `loop-counter` | `-` |
| CASE | `?: : ; ;:` | ```var ?: test1 : if var = test1 ; ... ;:``` | `test-variable` | `-` |
| SWAP | `swp` | `#1 #2 swp` | `a b` | `b a` |
| DUPLICATE | `dup` | `#1 dup` | `a` | `a a` |
| OVER | `ovr` | `#1 #2 ovr` | `a b` | `a b a` |
| ROTATE | `rot` | `#1 #2 #3 rot` | `a b c` | `b c a` |
| SET | `set` | `#3 #0 set` | `value address` | `-` |
| GET | `get` | `#0 get` | `address` | `value` |
| FUNCTION | `: ;` | `: function-name ... ;` | `-` | `-` |

| What | Where | Description (`a` is value on top of stack)|
| - | - | - |
| `var` | `-` | |
| `drop` | `-` | Discard top value from stack |
| `ms` | `-` | Wait for `a` ms |
| `nl` | `-` | Prints newline (`\n`) |
| `."` | `-` | Prints everything until `"` |
| `d:` | CASE | Default case |
| `s.` | `-` | Print stack |
| `sf.` | `-` | Print stack as floats |
| `+ - / * ~ neg abs` | `-` | Math functions |
| `f+ f- f/ f*` | `-` | Float math functions |
| `f> f<` | `-` | Convert to and from float |
| `> < >= <= <> ==` | `-` | Comparision function |
| `>> << \| &` | `-` | Bitwise functions |
| `|| &&` | `-` | Logic comparisions |
| `inf` | `-` | Loop back to beginning |
| `pc` | `-` | Pushes current program counter on stack |
| `go` | `-` | Sets program counter to value `a` |
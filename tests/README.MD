
#  Testing 
We are in WorkDir of script.sh
```console
foo@bar:~$ ff main.c tests
tests/dir1/main.c
tests/dir2/main.c

```
```console
foo@bar:~$ ff "main.c" tests
tests/dir1/main.c
tests/dir2/main.c
```
> Note: `""` is required for use with ?/* rendering.
```console
foo@bar:~$ ff "main*" tests
tests/dir1/main.c
tests/dir3/main.c.o.d
tests/dir3/main.c.o
tests/dir2/main.c
```
```console
foo@bar:~$ ff "*main*" tests
tests/two_main.c
tests/dir1/main.c
tests/two_main.h
tests/dir3/main.c.o.d
tests/dir3/main.c.o
tests/dir2/main.c
```
```console
foo@bar:~$ ff "main?" tests
```

# Valgrind

```console
foo@bar:~$ valgrind -s  --track-origins=yes  ./better_name "*main*" .
==31114== HEAP SUMMARY:
==31114==     in use at exit: 0 bytes in 0 blocks
==31114==   total heap usage: 343 allocs, 343 frees, 3,886,482 bytes allocated
==31114==
==31114== All heap blocks were freed -- no leaks are possible
==31114==
==31114== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
```console
foo@bar:~$ valgrind -s  --track-origins=yes  ./better_name "*main*" ~
==31247== HEAP SUMMARY:
==31247==     in use at exit: 0 bytes in 0 blocks
==31247==   total heap usage: 57,153 allocs, 57,153 frees, 1,168,747,127 bytes allocated
==31247== 
==31247== All heap blocks were freed -- no leaks are possible
==31247== 
==31247== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
```console
foo@bar:~$ valgrind -s  --track-origins=yes  ./better_name "*main*"
==31271== HEAP SUMMARY:
==31271==     in use at exit: 0 bytes in 0 blocks
==31271==   total heap usage: 343 allocs, 343 frees, 3,861,905 bytes allocated
==31271== 
==31271== All heap blocks were freed -- no leaks are possible
==31271== 
==31271== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
valgrind --tool=memcheck --suppressions=valgrind_suppressions.supp --num-callers=25 --error-limit=no --leak-check=yes --show-reachable=yes python test.py 2> vgerr.txt

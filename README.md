Solves Rubik's Cube cross 

run by:
1. download files `solveme.py` and `crossme.dll`
2. run `solveme.py` from command line

python source [`solveme.py`]:

<pre>
from ctypes import cdll

is_white_top = False

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init(is_white_top)
crossme_dll.dll_rotate("D F' R2 F2 D2 R2 B2 D' L2 U2 L' F2 L B D2 L' D' B2");
crossme_dll.dll_solve_cross()
crossme_dll.dll_solve_f2l()
</pre>

output example:

<pre>
Rotate [Yellow top, Green front]:
  D F' R2 F2 D2 R2 B2 D' L2 U2 L' F2 L B D2 L' D' B2


cross solutions: [Yellow top, Green front]:
   R  F  D  L  F  R

14232088 combinations searched for 1 seconds
F2L solutions: [Yellow top, Green front]:

Solving pair:
   D  B  U2 B' D'

Solving pair:
   U  R2 F2 R  F2 R

Solving pair:
   L  R' U' L' U  R

Solving pair:
   B' U2 R' U' R  U' B
   
225623761 combinations searched for 12 seconds
</pre>

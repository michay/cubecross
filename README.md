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
crossme_dll.dll_rotate("B' L D2 B' L2 D2 F D2 L2 F' U2 L2 B2 D' L' D2 F R2 F U");
crossme_dll.dll_print_cube()
crossme_dll.dll_solve_cross()
</pre>

output example:

<pre>
Rotate [Yellow top, Green front]:
  B' L D2 B' L2 D2 F D2 L2 F' U2 L2 B2 D' L' D2 F R2 F U

[W = White; G = Green; R = Red; O = Orange; B = Blue; Y = Yellow]
       R B B
       Y Y O
       R B W

W O G  Y R O  G G R  Y Y B
O R O  B G R  Y O R  W B W
G G W  O G R  G W O  Y R O

       B W W
       Y W B
       Y G B


cross solutions: [Yellow top, Green front]:
length 5; B2 L' R' B' R2
length 5; L' R  F  D2 F'
length 5; B' R' B' L' R
length 6; U' B' R' B' L' R
length 5; R  L' F  D2 F'
length 6; R2 B2 R  B' U2 L2
length 6; U  B' R' B' L' R
length 5; D2 L' F' R  D2
length 6; U2 B' R' B' L' R
length 6; R' B2 U2 R  B  L2
length 6; F2 L' R  F  D2 F
length 6; L2 R  L  F  D2 F'
length 6; L  R  F  L2 B  D2
length 6; B  R' B  L' R  B2
length 6; F  R  F  L' D2 F2
length 7; F' B' U2 F  R' B' L'
length 7; D  B' L' B' D' L' R
length 7; D' B2 F' R' D  L' R2

9748436 combinations searched for 1 seconds
</pre>

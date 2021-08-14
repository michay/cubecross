Solves Rubik's cube cross 

python source:

from ctypes import cdll

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init()
crossme_dll.dll_rotate("U2 F B' U R2 B' L' D2 F R2 B D2 F' L2 D2 B2 R2 D2 F' U' B");
crossme_dll.dll_print_cube()
crossme_dll.dll_solve_cross()


output example:

<pre>
Rotate [White top, Green front]:
  U2 F B' U R2 B' L' D2 F R2 B D2 F' L2 D2 B2 R2 D2 F' U' B

[W = White; G = Green; R = Red; O = Orange; B = Blue; Y = Yellow]
       R G B
       W W O
       O Y O

Y B G  W R W  B Y R  Y O G
G O W  R G O  W R W  G B Y
R R G  Y B R  W B O  Y G W

       O Y B
       B Y O
       G R B


cross solutions after Z2: [Yellow top, Green front]:
   length 5; F' D U L2 F' [0 seconds]
   length 5; F' U D L2 F' [0 seconds]
   length 6; B F' D2 F2 D' F2 [0 seconds]
   length 6; B' F' U2 B2 D F2 [0 seconds]
   length 6; F' B D2 F2 D' F2 [2 seconds]
   length 6; F' B' U2 B2 D F2 [2 seconds]
   length 6; F' D L2 U F' L2 [2 seconds]
   ...
   length 6; U2 F' U D L2 F' [6 seconds]
combinations searched = 14645088; total time = 6 seconds
</pre>

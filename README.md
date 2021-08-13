Solves Rubik's cube cross but searching all possible options with BFS

output example:


Rotate [White top, Green front]: U2 F B' U R2 B' L' D2 F R2 B D2 F' L2 D2 B2 R2 D2 F' U' B

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


cross solutions after: [Yellow top, Green front]:
   length 5; F' D U L2 F' [0 seconds]
   length 5; F' U D L2 F' [0 seconds]
   length 6; B F' D2 F2 D' F2 [0 seconds]
   length 6; B' F' U2 B2 D F2 [0 seconds]
   length 6; F' B D2 F2 D' F2 [1 seconds]
   length 6; F' B' U2 B2 D F2 [1 seconds]
   length 6; F' D L2 U F' L2 [1 seconds]
   length 6; F' D U F L2 F' [1 seconds]
   length 6; F' D U F' L2 F' [1 seconds]
   length 6; F' D U F2 L2 F' [1 seconds]
combinations searched = 6487593; total time = 1 seconds

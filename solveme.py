from ctypes import cdll

is_white_top = False

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init(is_white_top)
crossme_dll.dll_rotate("B U D' R' F L' F D2 B U2 R2 F2 B2 U2 B2 U B2 L2 U2 R");
crossme_dll.dll_solve_cross()
#crossme_dll.dll_rotate("U2 B2 F R' D F");
crossme_dll.dll_solve_f2l()



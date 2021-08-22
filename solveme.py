from ctypes import cdll

is_white_top = False

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init(is_white_top)
crossme_dll.dll_rotate("D F' R2 F2 D2 R2 B2 D' L2 U2 L' F2 L B D2 L' D' B2");
crossme_dll.dll_solve_cross()
crossme_dll.dll_solve_f2l()



from ctypes import cdll

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init()

crossme_dll.dll_rotate("U F2 B2 D2 F2 L2 D' R2 D' R2 F' D F U B R U2 D2 R F'");

crossme_dll.dll_print_cube()

crossme_dll.dll_solve_cross()



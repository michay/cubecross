from ctypes import cdll

crossme_dll = cdll.LoadLibrary('crossme\Release\crossme.dll')
crossme_dll.dll_init()

crossme_dll.dll_rotate("F2 D2 L2 D B2 U R2 D B2 R2 D2 R' D B' U' R U' F' U R' D");

crossme_dll.dll_print_cube()

crossme_dll.dll_solve_cross()



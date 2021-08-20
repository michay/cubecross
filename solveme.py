from ctypes import cdll

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init()
crossme_dll.dll_rotate("B' L D2 B' L2 D2 F D2 L2 F' U2 L2 B2 D' L' D2 F R2 F U");
crossme_dll.dll_print_cube()
crossme_dll.dll_solve_cross()



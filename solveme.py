from ctypes import cdll

crossme_dll = cdll.LoadLibrary('crossme.dll')
crossme_dll.dll_init()
crossme_dll.dll_rotate("F2 U B2 L2 F2 L2 B2 U D' B' L D L' U D' B2 R U' B2");
crossme_dll.dll_print_cube()
crossme_dll.dll_solve_cross()



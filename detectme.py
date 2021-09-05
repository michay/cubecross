import cv2
import ctypes
import numpy

def update_frame(frame):
   
   global crossme_dll

   hh = len(frame)
   ww = len(frame[0])

   crossme_dll.modify_frame(frame.ctypes.data_as(ctypes.POINTER(ctypes.c_uint32)), ww, hh)
   '''
   func = crossme_dll.modify_frame
   func.argtypes = [ctypes.POINTER(ctypes.c_char), ctypes.c_uint, ctypes.c_uint]
   
   data = frame.astype(numpy.c_char)
   data_p = data.ctypes.data_as(ctypes.POINTER(ctypes.c_char))

   func(data_p, ww, hh)
   '''
   
   
   
   '''
   '''

crossme_dll = ctypes.cdll.LoadLibrary('./crossme_x64.dll')

cap = cv2.VideoCapture()
# The device number might be 0 or 1 depending on the device and the webcam
cap.open(0, cv2.CAP_DSHOW)
while(True):
   ret, frame = cap.read()
   update_frame(frame)
   
   cv2.imshow('frame', frame)
   if cv2.waitKey(1) & 0xFF == ord('q'):
      break
      
cap.release()
cv2.destroyAllWindows()
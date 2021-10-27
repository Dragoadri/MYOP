# distutils: language = c++
# cython: language_level = 3
from libcpp cimport bool
from libcpp.string cimport string
import numpy as np
cimport numpy as cnp
from cython.view cimport array
from libc.string cimport memcpy
from framereader_pxd cimport FrameReader as cpp_FrameReader

cdef class FrameReader:
  cdef cpp_FrameReader* fr

  def __cinit__(self):
    self.fr = new cpp_FrameReader()

  def __dealloc__(self):
    del self.fr

  def load(self, file):
    return self.fr.load(file)

  @property
  def rgbSize(self):
    return self.fr.getRGBSize()
  
  @property
  def yuvSize(self):
    return self.fr.getYUVSize()
  
  @property
  def frameCount(self):
    return self.fr.getFrameCount()

  def get(self, id):
    addr = self.fr.get(id)
    cdef cnp.ndarray dat = np.empty(self.rgbSize, dtype=np.uint8)
    cdef char[:] dat_view = dat
    memcpy(&dat_view[0], addr, self.rgbSize)
    return dat


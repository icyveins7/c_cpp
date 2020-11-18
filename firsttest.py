# -*- coding: utf-8 -*-
"""
Created on Mon Jan 13 00:36:20 2020

@author: Seo
"""

import os
import numpy as np
import matplotlib.pyplot as plt
import scipy as sp
import scipy.signal as sps

def makeFreq(l, fs):
    freq = np.arange(l) * fs / l
    for i in range(len(freq)):
        if freq[i] >= fs/2:
            freq[i] = freq[i] - fs
            
    return freq

os.chdir('D:/UHD_install/lib/uhd/examples')
data = np.fromfile('usrp_samples.dat', dtype=np.int16).astype(np.float32).view(np.complex64)

h = plt.figure(1)
ax = h.add_subplot(211)
ax.plot(np.abs(data))
ax2 = h.add_subplot(212)
ax2.plot(makeFreq(len(data), 5e6), np.abs(sp.fftpack.fft(data)))
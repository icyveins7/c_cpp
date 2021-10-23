#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Oct 23 16:32:52 2021

@author: seolubuntu
"""

import uhd
import numpy as np

#%% Parameters
rxrate = 1e6
rxfreq = 1e9
lo_offset = 0


#%%
usrp = uhd.usrp.MultiUSRP()

usrp.set_rx_rate(rxrate)
rx_tune_req = uhd.types.TuneRequest(rxfreq, lo_offset)
usrp.set_rx_freq(rx_tune_req)

# Make streamer
cpu_fmt = "fc32"
otw_fmt = "sc16"
streamarg = uhd.usrp.StreamArgs(cpu_fmt, otw_fmt)
streamarg.channels = [0]
rxstream = usrp.get_rx_stream(streamarg)

# Make stream command
md = uhd.types.RXMetadata()
streamcmd = uhd.libpyuhd.types.stream_cmd(uhd.libpyuhd.types.stream_mode.start_cont)
streamcmd.stream_now = True

# Prepare buffer
data = np.zeros(10000, dtype=np.complex64)

# Send stream command
rxstream.issue_stream_cmd(streamcmd)

# Receive loop
for i in range(1000):
    rxstream.recv(data, md)
    
# Send stream command to end
streamcmd = uhd.libpyuhd.types.stream_cmd(uhd.libpyuhd.types.stream_mode.stop_cont)
rxstream.issue_stream_cmd(streamcmd)



set UHD_INSTALL_DIR=C:\uhd-4.0.0.0-install
set BOOST_DIR=C:\boost_1_75_0
cl rx_samples_to_file_buffered.cpp %UHD_INSTALL_DIR%\lib\uhd.lib /I"%UHD_INSTALL_DIR%\include" /EHsc /O2 /I"%BOOST_DIR%" /link "/LIBPATH:C:\boost_1_75_0\lib64-msvc-14.2"
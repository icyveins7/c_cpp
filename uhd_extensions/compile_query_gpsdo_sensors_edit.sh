mkdir bin
g++ query_gpsdo_sensors_edit.cpp -L$HOME/boost_1_72_0/lib -I$HOME/uhd-3.15.0.0-install/include -L$HOME/uhd-3.15.0.0-install/lib -luhd -I$HOME/boost_1_72_0/include -O2 -lboost_filesystem -lpthread -lboost_program_options -o bin/query_gpsdo_sensors_edit

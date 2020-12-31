# C++ application to interface with virtual CAN network

CAN C++ library obtained from [socketcan-cpp](https://github.com/siposcsaba89/socketcan-cpp).

## Setup the virtual CAN interface. 

Followed the tutorial [bring vcan interface up](https://elinux.org/Bringing_CAN_interface_up) using Ubuntu 18.04. 

Upon successful installation of virtual CAN network by running


`ifconfig`

The output should show the vcan interface up as seen here:

<img src="https://github.com/teresama/indoor_loc/blob/master/images/explanation_vcan.png?raw=true" width="280">


Once the interface is up, it is time to compile the C++ app.
To do so follow the commands:

`mkdir build`
`cd build`
`cmake ..`
`make`

Once the build is complete, it can be run for example the read_write.cpp app:

`./read_write`

Leading to the following output:

<img src="https://github.com/teresama/indoor_loc/blob/master/images/showcast.png?raw=true" width="280">

More socket CAN reference pointers:
[How_to_configure_and_use_CAN_bus](https://developer.ridgerun.com/wiki/index.php/How_to_configure_and_use_CAN_bus)
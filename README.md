Detailed instruction for software compile, start and testing with docker and docker compose

Documentation about the tasks, design and tests are found in "Documents/Protokoll_Kehr_Krzyszton" File.

## :rocket: Table of contents
* [System requirements](#system-requirements)
* [Recommended system](#recommended-system)
* [To start](#to-start)
* [To stop](#to-stop)
* [To reset DB](#to-reset-db)
* [Tested on](#tested-on)


## System requirements
```c++
Windows 10 or Ubuntu 18.04 (or higher) or macOS 10.15 (or higher)
Dual core procesor
8GB RAM
16GB free disk space
```


## Recommended system
```c++
Windows 10 or Ubuntu 18.04 (or higher) or macOS 10.15 (or higher)
Quad core procesor
16GB RAM
16GB free disk space

```

## To start: 
```c++
1. Clone repo
2. Switch to docker directory (one with docker-compose.yaml file in it)
3. Start Docker for Windows* or systemd on linux
4. Execute run.ps1 or run.sh (you may need sudo)
```

Data information are available on localhost:80 (Central 1) and localhost:81 (Central 2)
(Watch out for other applications listing on port 80 e.g. httpd, nginx)

to get the values of one specific Sensor please use:
```c++
localhost/avgSpeedSensor
         /fuelSensor
         /mileageSensor
         /trafficSensor
```

To check sensor status go to 
``` c++
localhost/status

```

## To stop:
```c++
Force quit
1. ctrl+c
2. optional (hit ctrl+c again to force quit)

Graceful shutdown
1. Open new terminal in the same directory
2. Execute stop.ps1 or stop.sh

```


## To reset DB:
```c++
1. Execute delete_data.ps1 or delete_data.sh

With the next run new drive for DBs will be created

```

## Tested on 
**Windows 10** with Docker for Windows and tested on following systems:

System 1
```
Intel i5 8250U 4 Core,
16GB RAM,
6GB HDD disk space used
```
System 2
```
AMD Ryzen 3600X 6 Core,
32GB RAM,
13GB SSD disk space used
```
System 3
```
AMD Ryzen 3600 6 Core,
32GB RAM,
6GB SSD disk space used
```
**Ubuntu 18.04** with Docker tested on following systems:

System 4
```
Intel i7 6500U 4 Core,
8GB RAM,
6GB HDD disk space used
```

****macOS Catalina 10.15****

System 5
```
Intel i5 2,7 GHz Dual-Core Intel Core i5
8GB RAM,
13GB HDD disk space used
```

*please use the WSL2 as described in the manual for better experience
https://docs.docker.com/docker-for-windows/wsl/
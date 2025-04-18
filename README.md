# Benternet

## Setup

### Installing ZMQ Library with Pacman and MSYS2 using Mingw.
- Open MSYS2.

- Copy and Paste :  
    ```cmd pacman -S mingw-w64-x86_64-zeromq```

- Now you can use the libary by added the following line in your program :  
    ```cpp #include "zmq.hpp"```

## **Broodjes** Service

### 1. Overview
- **Name of the Service:** Broodjes

- **Purpose:** The service will generate a random sandwich for you.

- **Use Case:** Usefull for when you can't decide what sandwich you want.

### 2. Architecture
- Client sends a msg starting with "Broodje?>" 
- Service will return "Broodje!>Example" as responds.

[ Client ] --- PUSH ----> [ Service ] ---> SUB ---> [ Client ]

### 3. Service Interface
- **Push :** 
    - Endpoint : ```tcp://benternet.pxl-ea-ict.be:24041```
    - Message Format : String
    - Example : ```Broodje?>```
- **Subscribe :** 
    - Endpoint : ```tcp://benternet.pxl-ea-ict.be:24042```
    - Message Format : String
    - Example : ```Broodje!>Mexicano```

### 4. 
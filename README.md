# Benternet - Broodjes Service

## Table of Contents
- [Overview](#overview)
- [Setup Instructions](#setup-instructions)
- [Architecture](#architecture)
- [Service Interface](#service-interface)
- [Usage](#usage)
- [Code Structure](#code-structure)
- [Sandwich Options](#sandwich-options-)
- [License](#license)

## Overview

**Project Name:** Benternet - Broodjes Service  
**Purpose:** This service generates a random sandwich ("broodje" in Dutch) when requested. It’s useful when you can't decide what to eat!  
**Use Case:** Ideal for fun apps, bots, or school projects where random suggestion services are demonstrated.

---

## Setup Instructions

### Prerequisites
- MSYS2 installed (for building C++ projects on Windows)
- A C++ compiler (MinGW recommended)
- ZeroMQ (ZMQ) library installed

### Installing ZMQ with MSYS2 and Pacman

1. Open **MSYS2 MSYS** terminal.
2. Install the ZeroMQ library by running:
   ```bash
   pacman -S mingw-w64-x86_64-zeromq
    ```
3. Include ZeroMQ in Your C++ Code
    Add the following include at the top of your program:
    ```cpp
    #include "zmq.hpp"
    ```

## Architecture
The service follows a simple publisher-subscriber (pub-sub) and push-pull model using ZeroMQ over TCP sockets.

Communication Flow
```css
[ Client ] --- PUSH ---> [ Broodjes Service ] --- SUB ---> [ Client ]
```
1. Client sends a message starting with **"Broodje?>"**.
2. Broodjes Service receives this and sends back a message starting with **"Broodje!>"**.
3. Client subscribes to receive the response.

## Service Interface

| **Role**   | **Endpoint**                                | **Type** |
|------------|----------------------------------------------|----------|
| Push       | `tcp://benternet.pxl-ea-ict.be:24041`        | PUSH     |
| Subscribe  | `tcp://benternet.pxl-ea-ict.be:24042`        | SUB      |


## Message Formats
1. Push Request (Client → Service)  
Format: **Broodje?>**   
Example:
    ```php
    Broodje?>
    ```

2. Subscribe Response (Service → Client)       
Format: **Broodje!>** SandwichName   
Example:
    ```cmd
    Broodje!>Broodje Mexicano
    ```
## New supported commands
| **Command**           | **Description**                                                               |
| --------------------- | ----------------------------------------------------------------------------- |
| `Broodje?>List`       | Lists all available sandwiches with prices and today's discount               |
| `Broodje?>Vote <num>` | Vote for a sandwich by index number                                           |
| `Broodje?>Top`        | Shows the top voted sandwiches                                                |
| `Broodje?>Buy <name>` | Simulate buying a sandwich (applies discount if it's the daily discount item) |

## Usage
Usage
Running the Service
Compile and run the provided C++ code.
It will:
- Listen for incoming sandwich-related requests.

- Reply with the appropriate message (random, list, vote, buy, etc.).

- Print logs to the console for debugging.
### Example Workflow  
```cmd
[Pusher] Enter message to send (or type 'exit' to quit): Broodje?>List
[Subscriber] Received: Broodje?>List
[Subscriber] Sent response: Broodje!>Sandwich List (Today's discount: Broodje Viandel - 15% off)
0: Broodje Mexicano (3.5 Euro)
1: Broodje Viandel (3.0 Euro)
```
You can also manually send messages through the command line using the Pusher thread.

## New Features
**Daily Discount**    
- Each day, one sandwich is selected for a random discount between 5% and 20%.
- Visible in the sandwich list and applied automatically when buying.

**Voting System**   
- Users can vote for their favorite sandwich.
- Votes are saved to a file (votes.txt) and survive restarts.

**Top Voted Sandwiches**
- The Broodje?>Top command returns the most voted sandwiches in descending order.

**Simulated Buying**
- Simulate a sandwich purchase and get the final price (including any discount).

## Code Structure

The service is multi-threaded and consists of:

### 1. `Subscriber Thread`
- Listens for "Broodje?>" messages.
- Processes commands like List, Vote, Buy, and Top.
- Replies using a PUSH socket.

### 2. `Pusher Thread`
- Allows sending test messages manually from console.
- Also displays the current day (used for discount logic).

### 3. `Listener Thread`
- Listens to unrelated topics like `"beer!>"` for demo purposes.
- Can be extended to support other services.

---

## Sandwich Options

Currently supported sandwiches:

- Broodje Mexicano
- Broodje Viandel
- Broodje Boulet
- Broodje Cervela
- Broodje Gezond
- Broodje Bakpao
- Bicky

Each sandwich has a defined price in Euro and may be discounted based on the day.

## License
This is a student project created for educational purposes.      
Feel free to use and modify it in your own projects.


# Solution Concept for the Airplane Synchronization Problem at the Airport

## Problem Description
Planes land and take off at the airport. To do so, they need exclusive access to the runway. The airport has a limited capacity and can accommodate a fixed number N of planes. If the number of planes at the airport is less than K (K < N), landing planes have priority for the runway.

## Task Objective
The goal is to implement a program that synchronizes planes' access to the runway and monitors the number of planes at the airport.

## Solution Concept

### Resources
1. Runway - synchronized using a mutex.
2. Space at the airport - synchronized using a mutex.
3. Conditions to signal runway availability and space availability at the airport.

### Data Structures
1. `airport_num`: Number of planes currently at the airport.
2. `takeoff_waiting_num`: Number of planes waiting to take off.
3. `landing_waiting_num`: Number of planes waiting to land.
4. Constants `K` and `N`: Limits related to the number of planes and spaces at the airport.

### Threads
1. **Landing Thread:**
   - Planes waiting to land increment the landing waiting counter.
   - If the airport is full or more than one plane is waiting to land, the plane waits for a signal about space availability.
   - After gaining access, the plane lands, increments the number of planes at the airport, and signals runway availability.

2. **Takeoff Thread:**
   - Planes waiting to take off increment the takeoff waiting counter.
   - The plane waits for runway access.
   - After gaining access, the plane takes off, decrements the number of planes at the airport and the number of planes waiting to take off.
   - If the number of planes at the airport is less than N, it signals space availability.

## Implementation
The above concept is implemented in C using threads and synchronization mechanisms such as mutexes and condition variables.
# Name: Halimah Abdulrasheed Instructor: Franco Carlacci Course: CS3790 Assignment 4  Date: 11/23/2024
Game of Life with Threads

Overview: 
This is a multithreaded implementation of Conway's Game of Life using C++. The program divides the game board into sections and processes each section in parallel using threads for improved performance. The interface uses NCurses for interactive cell selection and display.

Compiling the Program: 
On Ubuntu, make sure ncurses and pthread are installed before compiling the program. To check/install them: sudo apt-get update
                                                    sudo apt-get install g++ libncurses5-dev 
To compile the program: g++ -std=c++11 game_of_life.cpp -o GOL -pthread -lncurses

Running the Program: 
./GOL M N MAX
Where:
M: Board size (MxM grid); N: Number of sections per dimension (N²  threads will be created) ; MAX: Number of generations to simulate
Example:
./GOL 25 5 100  --> This creates a 25x25 board, uses 25 threads (5x5), and runs for 100 generations.

Interactive Controls: 
Initial Setup:
Left click: Toggle cell state (alive/dead)
Enter: Start simulation
Q: Quit program
During Simulation:
The simulation will run automatically for the specified number of generations
Live cells are shown as 'X'
Dead cells are shown as blank spaces

Board Division: 
The program divides the board into N×N sections. For example, with N=5 on a 25x25 board:
Rows/Cols  0-4   5-9   10-14  15-19  20-24
0-4        T0    T1    T2     T3     T4
5-9        T5    T6    T7     T8     T9
10-14      T10   T11   T12    T13    T14
15-19      T15   T16   T17    T18    T19
20-24      T20   T21   T22    T23    T24
Where T0-T24 represent the 25 threads, each processing their respective section

Limitations: 
Board size (M) must be divisible by the number of sections (N)
Terminal size must be larger than the board size
Maximum board size limited by terminal dimensions

Error Handling: 
The program includes error handling for:
Invalid command line arguments
Invalid board dimensions
Mouse input errors
Memory allocation failures

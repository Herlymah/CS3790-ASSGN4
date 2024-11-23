#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ncurses.h>
#include <cstdlib>

class GameOfLife {
private:
    int M; // Board size (MxM)
    int N; // Number of sections per dimension (N*N threads total)
    int MAX; // Number of generations
    std::vector<std::vector<bool>> currentBoard;
    std::vector<std::vector<bool>> nextBoard;
    std::vector<std::thread> threads;
    std::mutex displayMutex;
    WINDOW* win;

    int countNeighbors(int row, int col) {
        int count = 0;
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (i == 0 && j == 0) continue;
                
                int newRow = (row + i + M) % M;
                int newCol = (col + j + M) % M;
                
                if (currentBoard[newRow][newCol]) count++;
            }
        }
        return count;
    }

    void computeSubregion(int startRow, int endRow, int startCol, int endCol) {
        for (int i = startRow; i < endRow; i++) {
            for (int j = startCol; j < endCol; j++) {
                int neighbors = countNeighbors(i, j);
                
                if (currentBoard[i][j]) {
                    nextBoard[i][j] = (neighbors == 2 || neighbors == 3);
                } else {
                    nextBoard[i][j] = (neighbors == 3);
                }
            }
        }
    }

    void initNCurses() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
        curs_set(0);  // Hide cursor
        
        // Create a centered window for the game board
        int startY = (LINES - M) / 2;
        int startX = (COLS - M) / 2;  // Using single char per cell now
        win = newwin(M + 2, M + 2, startY, startX);
        box(win, 0, 0);
    }

public:
    GameOfLife(int boardSize, int sections, int generations) 
        : M(boardSize), N(sections), MAX(generations) {
        if (M % N != 0) {
            throw std::invalid_argument("Board size must be divisible by number of sections");
        }
        
        currentBoard = std::vector<std::vector<bool>>(M, std::vector<bool>(M, false));
        nextBoard = std::vector<std::vector<bool>>(M, std::vector<bool>(M, false));
        
        initNCurses();
    }

    ~GameOfLife() {
        delwin(win);
        endwin();
    }

    void readInitialConfigWithMouse() {
        mvprintw(0, 0, "Click cells to toggle them alive/dead");
        mvprintw(1, 0, "Press ENTER to start simulation");
        mvprintw(2, 0, "Press 'q' to quit");
        refresh();

        MEVENT event;
        int ch;
        bool configuring = true;

        while (configuring) {
            ch = getch();
            
            if (ch == KEY_MOUSE && getmouse(&event) == OK) {
                // Convert mouse coordinates to board coordinates
                int boardY = event.y - (LINES - M) / 2 - 1;
                int boardX = event.x - (COLS - M) / 2 - 1;

                if (boardY >= 0 && boardY < M && boardX >= 0 && boardX < M) {
                    currentBoard[boardY][boardX] = !currentBoard[boardY][boardX];
                    displayBoard();
                }
            }
            else if (ch == '\n') {
                configuring = false;
            }
            else if (ch == 'q' || ch == 'Q') {
                throw std::runtime_error("Configuration cancelled");
            }
        }

        // Clear instructions
        move(0, 0);
        clrtoeol();
        move(1, 0);
        clrtoeol();
        move(2, 0);
        clrtoeol();
        refresh();
    }

    void displayBoard() {
        std::lock_guard<std::mutex> lock(displayMutex);
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                wmove(win, i + 1, j + 1);
                waddch(win, currentBoard[i][j] ? 'X' : ' ');
            }
        }
        wrefresh(win);
    }

    void run() {
        int sectionSize = M / N;

        for (int generation = 0; generation < MAX; generation++) {
            threads.clear();

            // Create threads for each subregion
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    int startRow = i * sectionSize;
                    int endRow = startRow + sectionSize;
                    int startCol = j * sectionSize;
                    int endCol = startCol + sectionSize;

                    threads.emplace_back(&GameOfLife::computeSubregion, this, 
                                      startRow, endRow, startCol, endCol);
                }
            }

            // Wait for all threads to complete
            for (auto& thread : threads) {
                thread.join();
            }

            currentBoard = nextBoard;
            displayBoard();
            
            // Display current generation number
            mvprintw(LINES-1, 0, "Generation: %d/%d", generation + 1, MAX);
            refresh();
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Wait for a key press after simulation ends
        mvprintw(LINES-2, 0, "Simulation completed! Press any key to exit.");
        refresh();
        getch();
    }

    int countLiveCells() {
        int count = 0;
        for (const auto& row : currentBoard) {
            for (bool cell : row) {
                if (cell) count++;
            }
        }
        return count;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " M N MAX\n";
        std::cerr << "M: Board size (MxM)\n";
        std::cerr << "N: Number of sections (N*N threads)\n";
        std::cerr << "MAX: Number of generations\n";
        return 1;
    }

    int M = std::atoi(argv[1]);
    int N = std::atoi(argv[2]);
    int MAX = std::atoi(argv[3]);

    try {
        GameOfLife game(M, N, MAX);
        game.readInitialConfigWithMouse();
        game.run();
    } catch (const std::exception& e) {
        endwin();  // Clean up ncurses
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
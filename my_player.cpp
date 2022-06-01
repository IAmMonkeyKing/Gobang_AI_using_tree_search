#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <vector>

enum SPOT_STATE {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

// ----- Evaluator ----- //

class Evaluator{
    //Get a map state and output its score
    public:
        Evaluator() {};
        Evaluator(std::vector<std::vector<int>> &board);
        int evaluate();
    private:
        void get_lines(std::vector<std::vector<int>> &board);
        int evaluate_line(std::vector<int> &line);
        std::vector<std::vector<int>> horizontal_lines = std::vector<std::vector<int>>(15); //for lines "-"
        std::vector<std::vector<int>> vertical_lines = std::vector<std::vector<int>>(15);   //for lines "|"
        std::vector<std::vector<int>> up_right_lines = std::vector<std::vector<int>>(30);   //for lines "/"
        std::vector<std::vector<int>> down_right_lines = std::vector<std::vector<int>>(30); //for lines "\"
};

// ----- Decision maker ----- //

class DecisionMaker{
    //find and fout the next step
    public:
        DecisionMaker(char **argv){
            int input;
            fin = std::ifstream(argv[1]);
            fout = std::ofstream(argv[2]);

            //get board
            fin >> player;            
            board = std::vector<std::vector<int>>(SIZE);
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < SIZE; j++) {
                    fin >> input;
                    board[i].push_back(input);
                }
            }
        }

        ~DecisionMaker(){
            fin.close();
            fout.close();
        }

        void find_next_step(){
            //Now as random
            srand(time(NULL));
            int x, y;
            // Keep updating the output until getting killed.
            while(true) {
                // Choose a random spot.
                int x = (rand() % SIZE);
                int y = (rand() % SIZE);
                if (board[x][y] == EMPTY) {
                    fout << x << " " << y << std::endl;
                    // Remember to flush the output to ensure the last action is written to file.
                    fout.flush();
                }
            }
        }
    private:
        int player;
        const int SIZE = 15;
        std::vector<std::vector<int>> board;
        std::ifstream fin;
        std::ofstream fout;
};

// ----- Function Declarations ----- //



// ----- Main Function ----- //

int main(int, char** argv) {
    std::cout << "start\n";
    DecisionMaker decision_maker(argv);
    decision_maker.find_next_step();
    return 0;
}

// ----- Function Implementations ----- //




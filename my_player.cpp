#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <vector>
#include <set>


// ----- Point ----- //

class Point{
    // cords are 0-14
    public:
        Point();
        Point(int x, int y);
        Point(const Point &p);
        Point operator+(const Point &rhs) const;
        Point operator-(const Point &rhs) const;
        bool operator<(const Point &rhs) const;
        int x;
        int y;
    private:
};

Point::Point(): x{0}, y{0} {};

Point::Point(int x, int y): x{x}, y{y} {};

Point::Point(const Point &p): x{p.x}, y{p.y} {};

Point Point::operator+(const Point &rhs) const{
    return Point(x + rhs.x, y + rhs.y);
}

Point Point::operator-(const Point &rhs) const{
    return Point(x - rhs.x, y - rhs.y);
}

bool Point::operator<(const Point &rhs) const{
    if(x == rhs.x){
        return y < rhs.y;
    }
    return x < rhs.x;
}

// ----- State Tree ----- //

class StateTreeNode{
    public:
        StateTreeNode();
        StateTreeNode(int player);
        StateTreeNode(Point placement, int player, StateTreeNode *parent);

        friend class DecisionMaker;
    private: 
        int player;
        Point placement;
        StateTreeNode *parent;
        std::vector<StateTreeNode> childs;
};

StateTreeNode::StateTreeNode(){};

StateTreeNode::StateTreeNode(int player):player{player}, parent{nullptr}{}

StateTreeNode::StateTreeNode(Point placement, int player, StateTreeNode *parent):
placement{placement}, player{player}, parent{parent} {}

// ----- Chess Board ----- //

class ChessBoard{
    public:
        ChessBoard() {};
        ChessBoard(int size, std::ifstream &fin);
        void add_piece(Point &point, int player);
        void delete_piece(Point &point);
        bool is_valid(Point &point) const;
        bool is_empty(Point &point) const;
        void add_piece(int x, int y, int player);
        void delete_piece(int x, int y);
        bool is_valid(int x, int y) const;
        bool is_empty(int x, int y) const;
        std::vector<int> &operator[](int i);

        friend class DecisionMaker;
    private:
        int SIZE;
        std::vector<std::vector<int>> board;
};

ChessBoard::ChessBoard(int size, std::ifstream &fin): SIZE{size}{
    int input;
    board = std::vector<std::vector<int>>(SIZE);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> input;
            board[i].push_back(input);
        }
    }
}

void ChessBoard::add_piece(Point &point, int player){
    board[point.x][point.y] = player;
}

void ChessBoard::delete_piece(Point &point){
    board[point.x][point.y] = 0;
}

bool ChessBoard::is_valid(Point &point) const{
    return point.x > 0 && point.x < SIZE && point.y > 0 && point.y < SIZE && board[point.x][point.y] == 0;
}

bool ChessBoard::is_empty(Point &point) const{
    return board[point.x][point.y] == 0;
}

void ChessBoard::add_piece(int x, int y, int player){
    board[x][y] = player;
}
void ChessBoard::delete_piece(int x, int y){
    board[x][y] = 0;
}

bool ChessBoard::is_valid(int x, int y) const{
    return x > 0 && x < SIZE && y > 0 && y < SIZE && board[x][y] == 0;
}

bool ChessBoard::is_empty(int x, int y) const{
    return board[x][y] == 0;
}

std::vector<int> &ChessBoard::operator[](int i){
    return board[i];
}

// ----- Decision maker ----- //

class DecisionMaker{
    //find and fout the next step
    public:
        DecisionMaker(char **argv);
        ~DecisionMaker();
        void find_next_step();
    private:
        void create_tree(StateTreeNode &node, int depth, int curr_playe);
        void get_all_possible_steps();
        void init_directions();

        //basic information
        int player;
        int enemy;
        const int DEPTH = 4; //need to be an even number
        const int SIZE = 15;
        //chess board
        ChessBoard board;
        //for tree
        StateTreeNode root;
        std::vector<Point> directions;
        std::set<Point> possible_step_set;
        //I/O
        std::ifstream fin;
        std::ofstream fout;
};

DecisionMaker::DecisionMaker(char **argv){
    fin = std::ifstream(argv[1]);
    fout = std::ofstream(argv[2]);

    //get board
    fin >> player;
    enemy = (player == 1) ? 2:1;            
    root = StateTreeNode(player);
    board = ChessBoard(SIZE, fin);
    init_directions();
}

DecisionMaker::~DecisionMaker(){
    fin.close();
    fout.close();
}

void DecisionMaker::find_next_step(){
    //create tree
    get_all_possible_steps();
    create_tree(root, 1, player);

    //use alpha-beta pruning to get next step

}

void DecisionMaker::create_tree(StateTreeNode &node, int depth, int curr_player){
    if(depth >= DEPTH)return;
    int next_player = (curr_player == 1) ? 2:1;
    //create childs
    for(auto possible_step : possible_step_set){
        node.childs.push_back(StateTreeNode(possible_step, next_player, &node));
    }

    //go through all the childs to let them generate there own childs
    for(auto child:node.childs){
        std::set<Point> temp;
        
        //update board
        board.add_piece(child.placement, curr_player);
        
        //update possible step set
        for(auto delta_distanse:directions){
            Point possible_point = child.placement + delta_distanse;
            if(possible_step_set.find(possible_point) == possible_step_set.cend() && board.is_valid(possible_point)){
                possible_step_set.insert(possible_point);
                //record the changes
                temp.insert(possible_point);
            }
        }

        //let child generate it's own childs
        create_tree(child, depth+1, next_player);

        //reset board
        board.delete_piece(child.placement);

        //reset possible step set
        for(auto point:temp){
            possible_step_set.erase(point);
        }
    }
}

void DecisionMaker::get_all_possible_steps(){
    //all the spaces 2 away from a existing chess piece is possble next move
    bool found = false;
    for(int x = 0; x < SIZE; x++){
        for(int y = 0; y < SIZE; y++){
            if(!board.is_empty(x, y)){
                found = true;
                Point curr_point = Point(x, y);
                for(auto delta_distanse:directions){
                    Point possible_point = curr_point + delta_distanse;
                    if(board.is_valid(possible_point)){
                        possible_step_set.insert(possible_point);
                    }
                }
            }
        }
    }

    //if the chess board is empty
    if(!found){
        possible_step_set.insert(Point(7, 7));
    }
}

void DecisionMaker::init_directions(){
    directions.push_back(Point(-2, 2));
    directions.push_back(Point(-1, 2));
    directions.push_back(Point(0, 2));
    directions.push_back(Point(1, 2));
    directions.push_back(Point(2, 2));
    
    directions.push_back(Point(-2, 1));
    directions.push_back(Point(-1, 1));
    directions.push_back(Point(0, 1));
    directions.push_back(Point(1, 1));
    directions.push_back(Point(2, 1));

    directions.push_back(Point(-2, 0));
    directions.push_back(Point(-1, 0));
    directions.push_back(Point(1, 0));
    directions.push_back(Point(2, 0));

    directions.push_back(Point(-2, -1));
    directions.push_back(Point(-1, -1));
    directions.push_back(Point(0, -1));
    directions.push_back(Point(1, -1));
    directions.push_back(Point(2, -1));

    directions.push_back(Point(-2, -2));
    directions.push_back(Point(-1, -2));
    directions.push_back(Point(0, -2));
    directions.push_back(Point(1, -2));
    directions.push_back(Point(2, -2));
}

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

// ----- Main Function ----- //

int main(int, char** argv) {
    DecisionMaker decision_maker(argv);
    decision_maker.find_next_step();
    return 0;
}
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <array>
#include <vector>
#include <set>
#include <limits>
#include <string>


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

        friend std::ostream& operator<<(std::ostream& os, const Point& p);

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

std::ostream& operator<<(std::ostream& os, const Point& p){
    os << '(' << p.x << ", " << p.y << ')';
    return os;
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
        float value;
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
        void print() const;
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

void ChessBoard::print() const{
    std::cout << "---- Chess Board ----" << std::endl;
    std::cout << "  ";
    for(int i = 0; i < SIZE; i++){
        std::cout << i%10 << ' ';
    }
    std::cout << std::endl;
    for(int x = 0; x < SIZE; x++){
        std::cout << x%10 << ' ';
        for(int y = 0; y < SIZE; y++){
            if(board[x][y] == 0){
                std::cout << '.' << ' ';
            }
            else{
                std::cout << board[x][y] << ' ';
            }
        }
        std::cout << std::endl;
    }
}

// ----- Evaluator ----- //
enum SITUATION{
    WIN5 = 0, //OOOOO
    LIVE4 = 1, // .OOOO.
    OPEN4 = 2, // XOOOO. or .O.OOO. or OO.OO one step to winning
    LIVE3 = 3, //.OOO. or .O.OO.
    OPEN3 = 4, // XOOO. or .O.OOX or .OO.OX
};

class PlayerScore{
    public:
        PlayerScore();
        PlayerScore(int size, int player);
        friend class Evaluator;
    private:
        int SIZE;
        int player;
        std::vector<int> situation_occurence; 
};

PlayerScore::PlayerScore(int size, int player): SIZE{size}, player{player} {
    situation_occurence = std::vector<int>(5, 0);
};

class Evaluator{
    //Get a map state and output its score
    public:
        Evaluator() {};
        Evaluator(int size, int player);
        float evaluate(const std::vector<std::vector<int>> &board);
    private:
        bool evaluate_piece(int x, int y, const std::vector<std::vector<int>> &board);
        std::string get_piece_string(int O, int X, int piece);
        int player;
        int SIZE;
        std::set<std::string> win5_set;
        std::set<std::string> live4_set;
        std::set<std::string> open4_set;
        std::set<std::string> live3_set;
        std::set<std::string> open3_set;
        PlayerScore player1_score;
        PlayerScore player2_score;
};

Evaluator::Evaluator(int size, int player): SIZE{size}, player{player}{
    // OOPOO
    win5_set.insert("OOOOO");
    
    // .OPOO.
    live4_set.insert(".OOOO.");

    // XOPOO
    open4_set.insert("XOOOO.");
    open4_set.insert(".OOOOX");

    // O.POO
    open4_set.insert("O.OOO.");
    open4_set.insert("O.OOOX");
    // OOP.O
    open4_set.insert("XOOO.O");
    open4_set.insert(".OOO.O");

    // OP.OO
    open4_set.insert(".OO.OO");
    open4_set.insert("XOO.OO");

    // OPO
    live3_set.insert("..OOO..");
    live3_set.insert("X.OOO..");
    live3_set.insert("..OOO.X");
    live3_set.insert("X.OOO.X");

    // O.PO
    live3_set.insert(".O.OO..");
    live3_set.insert(".O.OO.X");
    // OP.O
    live3_set.insert("..OO.O.");
    live3_set.insert("X.OO.O.");

    // XOOP..
    open3_set.insert("XOOO...");
    open3_set.insert("XOOO..X");
    // ..POOX
    open3_set.insert("X..OOOX");
    open3_set.insert("...OOOX");

    // XOP.O.
    open3_set.insert("XXOO.O.");
    open3_set.insert(".XOO.O.");
    // .O.POX
    open3_set.insert(".O.OOXX");
    open3_set.insert(".O.OOX.");
    
    // XO.POO.
    open3_set.insert("XO.OO..");
    open3_set.insert("XO.OO.X");
    // .OP.OX
    open3_set.insert("..OO.OX");
    open3_set.insert("X.OO.OX");

    // O..PO
    open3_set.insert("O..OOXX");
    open3_set.insert("O..OOX.");
    open3_set.insert("O..OO.X");
    open3_set.insert("O..OO..");
    // OP..O
    open3_set.insert("XXOO..O");
    open3_set.insert("X.OO..O");
    open3_set.insert(".XOO..O");
    open3_set.insert("..OO..O");

    // O.O.O
    open3_set.insert(".O.O.O.");
    open3_set.insert("XO.O.O.");
    open3_set.insert(".O.O.OX");
    open3_set.insert("XO.O.OX");

    // X.OOO.X
    open3_set.insert("X.OOO.X");
};

float Evaluator::evaluate(const std::vector<std::vector<int>> &board){
    player1_score = PlayerScore(SIZE, 1);
    player2_score = PlayerScore(SIZE, 2);

}

bool Evaluator::evaluate_piece(int x, int y, const std::vector<std::vector<int>> &board){
    //update player score # return true if wins (have win5)
    PlayerScore *curr_player;
    int O, X;

    //init curr player information
    if(board[x][y] == 1){
        curr_player = &player1_score;
        O = 1;
        X = 2;
    }
    else{
        curr_player = &player2_score;
        O = 2;
        X = 1;
    }

    //horizontal
    std::string line;
    if(x - 2 > 0 && x + 2 < SIZE){
        //get 5 to check win5
        for(int i = -2; i <= 2; i++){
            line.append(get_piece_string(O, X, board[x+i][y]));
        }

        if(win5_set.find(line) != win5_set.cend()){
            //if have win5 game is over return
            curr_player->situation_occurence[WIN5]++;
            return true;
        }
        else if(x + 3 < SIZE){
            //get 6 to check open4 and live 4
            line.append(get_piece_string(O, X, board[x+3][y]));

            if(live4_set.find(line) != live4_set.cend()){
                curr_player->situation_occurence[LIVE4]++;
            }
            else if(open4_set.find(line) != open4_set.cend()){
                curr_player->situation_occurence[OPEN4]++;
            }
            else if(x - 3 > 0){
                //get 7 to check open3 and live 3
                line = get_piece_string(O, X, board[x-3][y]) + line;
                if(live3_set.find(line) != live3_set.cend()){
                    curr_player->situation_occurence[LIVE3]++;
                }
                else if(open3_set.find(line) != open3_set.cend()){
                    curr_player->situation_occurence[OPEN3]++;
                }
            }  
        }
    }

    //vertical
    
}

std::string get_piece_string(int O, int X, int piece){
    switch(piece){
        case 0:
            return ".";
        case 1:
            if(O == 1){
                return "O";
            }
            else{
                return "X";
            }
        case 2:
            if(O == 2){
                return "O";
            }
            else{
                return "X";
            }
    }
}

// ----- Decision maker ----- //

class DecisionMaker{
    //find and fout the next step
    public:
        DecisionMaker(char **argv);
        ~DecisionMaker();
        void find_next_step();
        void print_possible_steps() const;
        void print_tree(StateTreeNode &node) const;
    private:
        void create_tree(StateTreeNode &node, int depth, int curr_player);
        void get_all_possible_steps();
        void init_directions();
        float alpha_beta_pruning(StateTreeNode &node, int depth, float alpha, float beta, bool is_player);
        
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
        Evaluator evaluator;
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
    evaluator = Evaluator(SIZE, player);
    root = StateTreeNode(player);
    board = ChessBoard(SIZE, fin);
    init_directions();

    std::cout << "Initail board" << std::endl;
    board.print();
}

DecisionMaker::~DecisionMaker(){
    fin.close();
    fout.close();
}

void DecisionMaker::find_next_step(){
    //create tree
    get_all_possible_steps();
    print_possible_steps();  
    create_tree(root, 1, player);

    //use alpha-beta pruning to get next step
    float final_value = alpha_beta_pruning(root, 1, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), true);
    std::cout << "Final_value : " << final_value << std::endl;
    float max_value = -std::numeric_limits<float>::max();
    for(auto &child:root.childs){
        //std::cout << "child value : " << child.value << " x : " << child.placement.x << " y : " << child.placement.y << std::endl;
        max_value = std::max(child.value, max_value);
        if(max_value == child.value){
            fout << child.placement.x << ' ' << child.placement.y << std::endl;
        }
    }
}

void DecisionMaker::print_possible_steps() const{
    std::cout << "----possble next steps----" << std::endl;
    for(auto step : possible_step_set){
        std::cout << step << ' ';
    }
    std::cout << std::endl;
}

void DecisionMaker::print_tree(StateTreeNode &node) const{
    std::cout << "New Level" << std::endl;
    for(auto child:node.childs){
        std::cout << child.placement << ' ';
    }
    std::cout << std::endl;
    for(auto child:node.childs){
        print_tree(child);
    }
}

void DecisionMaker::create_tree(StateTreeNode &node, int depth, int curr_player){
    if(depth >= DEPTH)return;
    int next_player = (curr_player == 1) ? 2:1;
    
    //create childs
    for(auto possible_step : possible_step_set){
        node.childs.push_back(StateTreeNode(possible_step, next_player, &node));
    }

    //go through all the childs to let them generate there own childs
    for(auto &child:node.childs){
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
        possible_step_set.erase(child.placement);

        //let child generate it's own childs
        create_tree(child, depth+1, next_player);

        //reset board
        board.delete_piece(child.placement);

        //reset possible step set
        for(auto point:temp){
            possible_step_set.erase(point);
        }
        possible_step_set.insert(child.placement);
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

float DecisionMaker::alpha_beta_pruning(StateTreeNode &node, int depth, float alpha, float beta, bool is_player){
    if(depth >= DEPTH){
        node.value = evaluator.evaluate(board.board);
        return node.value;
    }
    if(is_player){
        float value = -std::numeric_limits<float>::max();
        for(auto &child:node.childs){
            board.add_piece(child.placement, DecisionMaker::player);
            value = std::max(value, alpha_beta_pruning(child, depth+1, alpha, beta, false));
            board.delete_piece(child.placement);
            alpha = std::max(alpha, value);
            if(alpha >= beta){
                //beta will have the memory of all the child form the node parents(siblings)
                //if alpha is bigger means that the player will have better score at this path
                //so the enemy won't choose this path
                break;
            }
        }
        node.value = value;
        return value;
    }
    else{
        //for enemies turn the smaller the points the better
        float value = std::numeric_limits<float>::max();
        for(auto &child:node.childs){
            board.add_piece(child.placement, enemy);
            value = std::min(value, alpha_beta_pruning(child, depth+1, alpha, beta, true));
            board.delete_piece(child.placement);
            beta = std::min(beta, value);
            if(beta <= alpha){
                //alpha will have the memory of the nodes siblings
                //if beta is small means the player won't want this path
                //cause the enemy can go to a better board, compared to the other sibling paths in the tree that is visited before
                break;
            }
        }
        node.value = value;
        return value;
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


// ----- Main Function ----- //

int main(int, char** argv) {
    std::cout << "in program" << std::endl;
    DecisionMaker decision_maker(argv);
    std::cout << "decision_maker built" << std::endl;
    decision_maker.find_next_step();
    std::cout << "finish findng next step" << std::endl;
    return 0;
}
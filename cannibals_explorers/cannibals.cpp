/*
Compiling Instructions:
g++ -std=c++17 -O2 *.cpp -o cannibals

Includes / Dependencies:
- <deque> : double-ended queues
- <vector> : dynamic arrays
- <set> : ordered unique collections
- <iomanip> : formatted output
- <iostream> : input/output streams
*/


#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

constexpr int SHORE_SIZE = 6;           // slots per bank
constexpr int BOAT_CAPACITY = 2;        // max in boat
constexpr int MAX_MOVES = 5;            // max possible moves
constexpr int TOTAL_CANNIBALS = 3;      // total amount of cannibals
constexpr int TOTAL_EXPLORERS = 3;      // total amount of explorers
constexpr int PADDING = 30;             // Padding amount for output
constexpr int BOAT_PADDING = 4;         // Padding for, 4 chars needed
constexpr int CENTER_PADDING = (PADDING + (SHORE_SIZE * 2))/2;  // Padding amount for center

enum Shore { LEFT, RIGHT };

// move struct is just POD  will be created using count for each role

struct Move
{  
    int cannibalCount_;  
    int explorerCount_;

    // only one constructor, since we will always create a move with both variables
    Move(int cannibals, int explorers) : 
    cannibalCount_(cannibals),  explorerCount_(explorers) 
    {  }
};


// will keep track of the current state, amount of each role on each side and location of boat.
class State
{
    int leftCannibals_{TOTAL_CANNIBALS};
    int leftExplorers_{TOTAL_EXPLORERS}; 
    int rightCannibals_{0};
    int rightExplorers_{0};
    int boatSide_{LEFT};

public: 

    // returns current boat side
    int getBoatSide() const { return boatSide_; }

    // checks if success criteria is met, this case everyone transported to other side.
    bool isSuccess() const 
    { 
        return (rightCannibals_ == TOTAL_CANNIBALS && rightExplorers_ == TOTAL_EXPLORERS); 
    }

    // checks if position is valid, cannibals cannot outnumber explorers on either side, 
    // except when there are no explorers on that side.
    bool isValid() const
    {
        return !((leftCannibals_ > leftExplorers_ && leftExplorers_ != 0) || 
                 (rightCannibals_ > rightExplorers_ && rightExplorers_ != 0));
    }


    // applies move to current state, subtracts from current boat side, and adds to the opposite boat side
    void applyMove(const Move& move)
    {
        if(boatSide_ == RIGHT)
        {
            leftExplorers_ += move.explorerCount_;
            leftCannibals_ += move.cannibalCount_;
            rightExplorers_ -= move.explorerCount_;
            rightCannibals_ -= move.cannibalCount_;
        }
        else 
        {
            leftExplorers_ -= move.explorerCount_;
            leftCannibals_ -= move.cannibalCount_;
            rightExplorers_ += move.explorerCount_;
            rightCannibals_ += move.cannibalCount_;
        }
        // flip boatSide_ between LEFT(0) and RIGHT(1) using XOR with 1
        boatSide_ ^= 1;
    }

    void undoMove(const Move& move)
    {
        // applying applyMove() twice with the same move restores the prior state
        // this symmetry holds because moves are deterministic and reversible
        applyMove(move); 
    }

    // generates possible moves in the current state
    std::vector<Move> createMoveList() const
    {
        // find which side we are generating moves
        int cannibals = (boatSide_ == LEFT ? leftCannibals_ : rightCannibals_);
        int explorers = (boatSide_ == LEFT ? leftExplorers_ : rightExplorers_);

        std::vector<Move> moveList;
        moveList.reserve(MAX_MOVES); // reserve space so we do not reallocate for more moves.


        /* 
        Since there is only two spots on boat, we do not care if each role is
        more than 2 but we need to know if its less, using std::min, we have a ceiling of 2, 
        but it can be lower if each role is 1 or 0, we can use nested loop to find each possible combinations
        and create the move and add it to our list.
        */
        for(int c = 0; c <= std::min(BOAT_CAPACITY, cannibals); ++c)
        {
            for(int e = 0; e <= std::min(BOAT_CAPACITY - c, explorers); ++e)
            {
                if(c + e > 0)
                {
                    moveList.emplace_back(Move(c, e));
                }
            }
        }
        return moveList;
    }

    // PRINT FUNCTIONS 

    // prints selected shore, by C or E and - if empty slots until 6.
    void printShore(int shore) const
    {
        int cannibals = (shore == LEFT ? leftCannibals_ : rightCannibals_);
        int explorers = (shore == LEFT ? leftExplorers_ : rightExplorers_);

        for(int i = 0; i < cannibals; i++) std::cout << 'C';
        for(int i = 0; i < explorers; i++) std::cout << 'E';
        for(int i = 0; i < SHORE_SIZE - (cannibals + explorers); i++) std::cout << '-';
    }

    // prints empty boat, next to the shore of selected side
    // uses std::setw for horizontal alignment of boat and shore graphics
    void printEmptyBoat(int shore) const
    {
        std::cout << (shore == LEFT ? std::setw(2) : std::setw(PADDING)) <<  "\\";
        for(int i = 0; i < BOAT_CAPACITY; i++) std::cout << '-';
        std::cout <<  "/" << (shore == LEFT ? std::setw(PADDING) : std::setw(2)); 
    }

    // prints boat in the middle of river, with the people inside depending on move
    void printBoat(const Move& move) const 
    {
        std::cout << "\n" << std::setw(CENTER_PADDING) << "\\"; // adjust horizontal alignment to center

        for(int i = 0; i < move.cannibalCount_; i++) std::cout << 'C';
        for(int i = 0; i < move.explorerCount_; i++) std::cout << 'E';
        for(int i = 0; i < BOAT_CAPACITY - (move.cannibalCount_ + move.explorerCount_); i++) std::cout << '-';

        std::cout << "/\n";
    }

    // defines lexicographical comparison (meaning first differing type will be compared) by tying member references into a tuple;
    // avoids manual comparisons or copies

    bool operator<(const State& other) const 
    {
        return std::tie(leftCannibals_, leftExplorers_, rightCannibals_, rightExplorers_, boatSide_) <
               std::tie(other.leftCannibals_, other.leftExplorers_, other.rightCannibals_, other.rightExplorers_, other.boatSide_);
    }
};



// solution class, will be incharge or recursion and printing the output, for 
class Solution
{
    State currentState_; // current state
    std::deque<Move> previousMoves; // stores correct in a deque for O(1) push_back and pop. allows for iteration after recursion
    std::set<State> visitedStates; // stores previous states in a hashset (no copies allowed), allowing us to see if we previously visited this state, if so we can skip infinte loops
    std::deque<Move> solutionMoves; // stores correct in a deque for O(1) push_back and pop. allows for iteration after recursion


public:

    void backtrack()
    {
        // base case: invalid position or already visited
        // emplace() returns {iterator, bool}; skip if insertion fails (state seen before)
        if(!currentState_.isValid() || !visitedStates.emplace(currentState_).second) 
        {
            return;
        }

        // success case, we can set our success variable to true, and unwind stack.
        if(currentState_.isSuccess())
        {
            if(solutionMoves.empty() || previousMoves.size() < solutionMoves.size())
            {
                solutionMoves = previousMoves;
            }
            return;
        }

        /* 
        create the move list of the current position and recursivly call each move
        first apply move, then add current move to the deque, 
        recursivly check if this position is successful/failed, or can go forward, until either one of the previous conditions are met
        if we find a better solution or its a new one we will save it to a seperate deque.
        if its not succesful we undo move, pop the move from deque, and check next move in vector.
        */
        
        for(const auto& move : currentState_.createMoveList())
        {
            currentState_.applyMove(move);
            previousMoves.push_back(move);
            backtrack();
            currentState_.undoMove(move);
            previousMoves.pop_back();
        }
    }


    // final outpout function, prints state and boat side, then on a new line boat and who is on it.
    // we want to perserve our current state so we can create a new state and use the correct moves on it this simulated state.
    void printOutput() const 
    {
        for(int i = 0; i < (PADDING + (SHORE_SIZE * 2) + BOAT_PADDING); i++)
        {
            std::cout << "-";
        }

        std::cout << "\nMoves In Order:\n\n";
        State simState;
        for (const auto& move : solutionMoves) 
        {
            //for each move first print the left bank then an empty boat location then right 
            //on each move the boat will be put in the middle with the two selecet people inside the boat
            simState.printShore(LEFT);
            simState.printEmptyBoat(simState.getBoatSide());
            simState.printShore(RIGHT);
            simState.printBoat(move);
            simState.applyMove(move);
        }
        //print final shore
        simState.printShore(LEFT);
        simState.printEmptyBoat(simState.getBoatSide());
        simState.printShore(RIGHT);
        std::cout << std::endl; //std::endl to flush
    }
};



int main()
{
    Solution solver;
    //run backtrack
    solver.backtrack();
    //print output from the backtrack
    solver.printOutput();    
    return 0;
}
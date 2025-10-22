/*
Knight's Tour using Warnsdorff-guided backtracking.
Author: Grigory Ermizin
Description:
   - Bitboard-based implementation
   - Warnsdorff heuristic ordering
 */


#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <deque>
#include <array>  
#include <string>

using Bitboard = uint64_t;
using MoveList = std::array<std::pair<int,int>, 8>;

constexpr Bitboard FullBB = 0xFFFFFFFFFFFFFFFFULL;
constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

enum Squares
{
    a1, b1, c1, d1, e1, f1, g1, h1, 
    a2, b2, c2, d2, e2, f2, g2, h2, 
    a3, b3, c3, d3, e3, f3, g3, h3, 
    a4, b4, c4, d4, e4, f4, g4, h4, 
    a5, b5, c5, d5, e5, f5, g5, h5, 
    a6, b6, c6, d6, e6, f6, g6, h6, 
    a7, b7, c7, d7, e7, f7, g7, h7, 
    a8, b8, c8, d8, e8, f8, g8, h8, noSq
};

constexpr bool getBit(const Bitboard& b, int sq)
{
    return b & (1ULL << sq);
}

constexpr void setBit(Bitboard& b, int sq)
{
    b |= (1ULL << sq);
}

constexpr int lsb(const Bitboard& b) //least significant bit, bit in the lowest position
{
    return __builtin_ctzll(b);
}

constexpr void popBit(Bitboard &b, int sq)
{
    b &= ~(1ULL << sq);
}

constexpr int popLsb(Bitboard &b) 
{
    int sq = lsb(b);
    b &= ~(1ULL << sq);
    return sq;
}

constexpr int bitCount(const Bitboard& b)
{
    return __builtin_popcountll(b);  
} 

constexpr int coordsToSq(int row, int column)
{
    return (row * 8) + column;
}

constexpr int getRow(int sq)
{
    return sq / 8;
}

constexpr int getColumn(int sq)
{
    return sq % 8;
}

void sqToString(int sq)
{
    if(sq == noSq)
    {
        std::cout << "NA"; return;
    }
    char c = 'a' + sq % 8;
    std::cout << c << sq / 8 + 1 << " ";   
}

class Backtrack
{
    Bitboard visitedSqs = {0ULL};
    std::deque<int> stackSq;

    bool fullTour{false};
    int totalMoves{0}; 
    int startingSq{noSq}; 

  
    Bitboard genMoveBoard(int sq)
    {
        //generate posible knight move bitboard from given square
        Bitboard bb = 1ULL << sq;
        Bitboard attacks = 0ULL;

        attacks |= ((~FileABB & bb) << 15);
        attacks |= ((~FileHBB & bb) << 17);
        attacks |= ((~(FileABB | FileBBB) & bb) << 6);
        attacks |= ((~(FileGBB | FileHBB) & bb) << 10);

        
        attacks |= ((~FileABB & bb) >> 17);
        attacks |= ((~FileHBB & bb) >> 15);
        attacks |= ((~(FileABB | FileBBB) & bb) >> 10);
        attacks |= ((~(FileGBB | FileHBB) & bb) >> 6);

        return attacks & ~visitedSqs; //exclude visited squares
    }

    MoveList genList(int sq)
    {
        MoveList moves;
        Bitboard attacks = genMoveBoard(sq);

        for(auto& item : moves)
        {
            item.first = noSq;
            item.second = 9;
        }

        // generate possible knight-move bitboard from given square
        // sort moves in ascending order of onward degree (Warnsdorff’s rule)
        int count = 0;
        while(attacks)
        {
            int sq = popLsb(attacks);
            std::pair<int,int> newSq = {sq, bitCount(genMoveBoard(sq))};
            int i = count - 1;
            while(i >= 0 && moves[i].second > newSq.second)
            {
                moves[i+1] = moves[i];
                --i;
            }
            moves[i + 1] = newSq;
            count++;
        }

        return moves;
    }

    void makeMove(int sq)
    {
        totalMoves++;
        setBit(visitedSqs, sq);
        stackSq.push_back(sq);

        if(visitedSqs == FullBB)
        {
            fullTour = true;
            return;
        }
            
        auto moves = genList(sq);

        if(moves[0].first == noSq) 
        { 
            popBit(visitedSqs, sq);
            stackSq.pop_back();
            return; 
        }

        int i = 0;
        while(moves[i].first != noSq)
        {
            makeMove(moves[i].first);
            if(fullTour) { return; }
            ++i; 
        }

        popBit(visitedSqs, sq);
        stackSq.pop_back();
    }

    void printBoard(Bitboard bb)
    {
        for(int row = 7; row >= 0; row--)
        {
            std::cout << row + 1 << " ";

            for(int column = 0; column <= 7; column++)
            {
                std::cout << (getBit(bb, coordsToSq(row, column)) ? '1' : '.') << " ";
            }
            std::cout << "\n";
        }

        std::cout << "  A B C D E F G H\n\n";
    }

public: 

    int getTotalMoves() const
    {
        return totalMoves;
    }
    
    void printStack() const
    {
        int i = 0;
        for (auto sq : stackSq) {
            std::cout << ++i << ": ";
            sqToString(sq);
            std::cout << "\n";
        }
    }

    void backtrack(int sq)
    {
        startingSq = sq;
        makeMove(sq);
    }

};


int main()
{

    std::cout << R"(  
        |\__/,|   (`\
      _.|o o  |_   ) )
    -(((---(((--------    
        KNIGHT’S TOUR
    )" << std::endl;

    std::string stringSq;
    int row, col, numSq;

    do {
        std::cout << "Please enter starting square (ex: a1): ";
        std::cin >> stringSq;

        if(stringSq.length() == 2 &&
        stringSq[0] >= 'a' && stringSq[0] <= 'h' &&
        stringSq[1] >= '1' && stringSq[1] <= '8') 
        {
            col = stringSq[0] - 'a';
            row = stringSq[1] - '1';  // convert '1'-'8' to 0-7
            numSq = coordsToSq(row, col);
            break;
        } 
        else 
        {
            std::cout << "Invalid square, try again.\n";
        }
    } while(true);

    Backtrack bt;

    std::cout << "\n\n\nStarting backtrack: \n";
    auto start = std::chrono::system_clock::now();
    bt.backtrack(numSq);
    auto end =  std::chrono::system_clock::now();


    int count = bt.getTotalMoves();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "\nTime taken: " << elapsed_seconds.count() << " seconds";
    std::cout << "\nNodes: " << count;
    std::cout << "\nNodes per second: " << count/elapsed_seconds.count();

    std::cout << "\n\nRoute: \n";
    bt.printStack();
    return 0;
}
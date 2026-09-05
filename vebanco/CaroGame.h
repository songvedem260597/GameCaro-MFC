#pragma once

// Portable game rules. No Windows/MFC types: the same code is used by the UI
// and by the regression tests. Rules: freestyle, five OR MORE in a row wins.
#include <cstdlib>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace caro {

enum Stone { Empty = 0, X = 1, O = 2 };

struct Move {
    int row, col;
    Move(int r = -1, int c = -1) : row(r), col(c) {}
    bool operator==(const Move& other) const {
        return row == other.row && col == other.col;
    }
};

class Game {
public:
    enum { MinSize = 5, MaxSize = 30, DefaultSize = 15 };

    explicit Game(int size = DefaultSize) { Reset(size); }

    void Reset(int size = DefaultSize) {
        if (size < MinSize || size > MaxSize)
            throw std::invalid_argument("Board size must be between 5 and 30.");
        size_ = size;
        winner_ = Empty;
        history_.clear();
        winningLine_.clear();
        for (int r = 0; r < MaxSize; ++r)
            for (int c = 0; c < MaxSize; ++c)
                cells_[r][c] = Empty;
    }

    int Size() const { return size_; }
    bool Inside(int r, int c) const {
        return r >= 0 && c >= 0 && r < size_ && c < size_;
    }
    Stone At(int r, int c) const {
        return Inside(r, c) ? cells_[r][c] : Empty;
    }
    Stone Turn() const { return history_.size() % 2 == 0 ? X : O; }
    Stone Winner() const { return winner_; }
    bool Draw() const {
        return winner_ == Empty && MoveCount() == size_ * size_;
    }
    bool Finished() const { return winner_ != Empty || Draw(); }
    int MoveCount() const { return static_cast<int>(history_.size()); }
    const std::vector<Move>& History() const { return history_; }
    const std::vector<Move>& WinningLine() const { return winningLine_; }

    bool Play(int r, int c) {
        if (Finished() || !Inside(r, c) || cells_[r][c] != Empty)
            return false;
        const Stone player = Turn();
        history_.push_back(Move(r, c));
        cells_[r][c] = player;
        static const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        for (int d = 0; d < 4; ++d) {
            const int dr = directions[d][0], dc = directions[d][1];
            const int before = Count(r, c, -dr, -dc, player);
            const int after = Count(r, c, dr, dc, player);
            if (1 + before + after >= 5) {
                winner_ = player;
                for (int k = -before; k <= after; ++k)
                    winningLine_.push_back(Move(r + k * dr, c + k * dc));
                break;
            }
        }
        return true;
    }

    // Every prefix of a legal game is non-terminal. Removing the last move
    // therefore clears both win and draw without recomputing the whole board.
    bool Undo() {
        if (history_.empty()) return false;
        const Move last = history_.back();
        cells_[last.row][last.col] = Empty;
        history_.pop_back();
        winner_ = Empty;
        winningLine_.clear();
        return true;
    }

    // In computer mode return to the human's previous decision, not O's turn.
    bool UndoTurn(bool computer) {
        if (history_.empty()) return false;
        const int count = computer && Turn() == X && MoveCount() >= 2 ? 2 : 1;
        for (int i = 0; i < count; ++i) Undo();
        return true;
    }

    // Deterministic, one-ply tactical opponent (not a minimax/search engine).
    // Take a win first, block an immediate loss next, then score open lines.
    // This function never changes the position and is also used for hints.
    Move SuggestMove() const {
        if (Finished()) return Move();
        if (history_.empty()) return Move(size_ / 2, size_ / 2);
        const Stone player = Turn(), opponent = player == X ? O : X;
        Move best;
        long long bestScore = -1;
        for (int r = 0; r < size_; ++r) {
            for (int c = 0; c < size_; ++c) {
                if (cells_[r][c] != Empty || !HasNeighbour(r, c)) continue;
                const int attack = Evaluate(r, c, player);
                const int defence = Evaluate(r, c, opponent);
                long long score;
                if (attack >= WinScore) score = 1000000000LL;
                else if (defence >= WinScore) score = 100000000LL;
                else score = static_cast<long long>(attack) * 10 +
                             static_cast<long long>(defence) * 11;
                // Centrality breaks ties without affecting tactical priorities.
                score = score * 1000 + 2 * size_ -
                    std::abs(2 * r - size_ + 1) - std::abs(2 * c - size_ + 1);
                if (score > bestScore) { bestScore = score; best = Move(r, c); }
            }
        }
        return best;
    }

    // Bounded, versioned, human-readable saves. Loading is transactional:
    // malformed input leaves BOTH this game and the caller's mode unchanged.
    bool Save(std::ostream& out, bool computer) const {
        out << "CARO 1\n" << size_ << ' ' << (computer ? 1 : 0)
            << ' ' << MoveCount() << '\n';
        for (std::size_t i = 0; i < history_.size(); ++i)
            out << history_[i].row << ' ' << history_[i].col << '\n';
        return static_cast<bool>(out);
    }

    bool Load(std::istream& in, bool& computer) {
        std::string magic;
        int version = 0, size = 0, mode = 0, count = 0;
        if (!(in >> magic >> version >> size >> mode >> count) ||
            magic != "CARO" || version != 1 || size < MinSize ||
            size > MaxSize || (mode != 0 && mode != 1) || count < 0 ||
            count > size * size) return false;
        Game loaded(size);
        for (int i = 0; i < count; ++i) {
            int r = -1, c = -1;
            if (!(in >> r >> c) || !loaded.Play(r, c)) return false;
        }
        in >> std::ws;
        if (in.bad() || !in.eof()) return false;
        *this = loaded;
        computer = mode == 1;
        return true;
    }

private:
    enum { WinScore = 1000000 };
    int size_;
    Stone cells_[MaxSize][MaxSize];
    Stone winner_;
    std::vector<Move> history_;
    std::vector<Move> winningLine_;

    int Count(int r, int c, int dr, int dc, Stone player) const {
        int count = 0;
        for (r += dr, c += dc; Inside(r, c) && cells_[r][c] == player;
             r += dr, c += dc) ++count;
        return count;
    }

    bool HasNeighbour(int r, int c) const {
        for (int dr = -2; dr <= 2; ++dr)
            for (int dc = -2; dc <= 2; ++dc)
                if (Inside(r + dr, c + dc) && cells_[r + dr][c + dc] != Empty)
                    return true;
        return false;
    }

    int Evaluate(int r, int c, Stone player) const {
        static const int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        int score = 0;
        for (int d = 0; d < 4; ++d) {
            const int dr = directions[d][0], dc = directions[d][1];
            const int before = Count(r, c, -dr, -dc, player);
            const int after = Count(r, c, dr, dc, player);
            const int length = 1 + before + after;
            if (length >= 5) return WinScore;
            const int r1 = r - (before + 1) * dr, c1 = c - (before + 1) * dc;
            const int r2 = r + (after + 1) * dr, c2 = c + (after + 1) * dc;
            const int open = (Inside(r1, c1) && At(r1, c1) == Empty ? 1 : 0) +
                             (Inside(r2, c2) && At(r2, c2) == Empty ? 1 : 0);
            if (open == 0) continue;
            if (length == 4) score += open == 2 ? 80000 : 14000;
            else if (length == 3) score += open == 2 ? 7000 : 700;
            else if (length == 2) score += open == 2 ? 300 : 40;
            else score += open == 2 ? 20 : 5;
        }
        return score;
    }
};

// Shared paint/hit-test geometry: half-open boundaries assign any grid line
// to at most ONE cell, and clicks outside the board can never place a stone.
struct BoardLayout {
    int left, top, cell, size;
    BoardLayout(int width, int height, int boardSize)
        : left(0), top(0), cell(0), size(boardSize) {
        if (boardSize < Game::MinSize || boardSize > Game::MaxSize) return;
        const int availableWidth = width - 40;
        const int availableHeight = height - 150;
        if (availableWidth <= 0 || availableHeight <= 0) return;
        cell = (availableWidth < availableHeight ? availableWidth : availableHeight) / size;
        if (cell < 8) { cell = 0; return; }
        left = (width - cell * size) / 2;
        top = 70 + (availableHeight - cell * size) / 2;
    }
    bool Hit(int x, int y, Move& move) const {
        if (cell == 0 || x < left || y < top ||
            x >= left + cell * size || y >= top + cell * size) return false;
        move = Move((y - top) / cell, (x - left) / cell);
        return true;
    }
};

} // namespace caro

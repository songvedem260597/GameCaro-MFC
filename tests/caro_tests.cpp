#include "../vebanco/CaroGame.h"
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace {
int checks = 0;
void Check(bool condition, const char* expression, int line) {
    ++checks;
    if (!condition) {
        std::ostringstream message;
        message << "Line " << line << ": " << expression;
        throw std::runtime_error(message.str());
    }
}
#define CHECK(expression) Check((expression), #expression, __LINE__)

std::string Snapshot(const caro::Game& game, bool mode = false) {
    std::ostringstream out;
    CHECK(game.Save(out, mode));
    return out.str();
}

void Rules() {
    caro::Game game;
    CHECK(game.Size() == 15);
    CHECK(game.Turn() == caro::X);
    CHECK(game.Winner() == caro::Empty);
    CHECK(!game.Finished());
    CHECK(!game.Undo());
    CHECK(!game.Play(-1, 0));
    CHECK(!game.Play(0, 15));
    CHECK(game.MoveCount() == 0);
    CHECK(game.Play(0, 0));
    CHECK(!game.Play(0, 0));
    CHECK(game.Turn() == caro::O);
    CHECK(game.Undo());
    CHECK(game.At(0, 0) == caro::Empty);
    bool threw = false;
    try { game.Reset(31); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
    CHECK(game.Size() == 15);

    // All four axes, including positions at the board's edges.
    const int dr[] = {0, 1, 1, 1}, dc[] = {1, 0, 1, -1};
    for (int d = 0; d < 4; ++d) {
        game.Reset();
        for (int k = 0; k < 5; ++k) {
            CHECK(game.Play(k * dr[d], (d == 3 ? 14 : 0) + k * dc[d]));
            if (k < 4) {
                CHECK(game.Winner() == caro::Empty); // four is NOT a win
                CHECK(game.Play(14, 2 * k));
            }
        }
        CHECK(game.Winner() == caro::X);
        CHECK(game.WinningLine().size() == 5);
        CHECK(!game.Play(10, 10));
        CHECK(game.Undo());
        CHECK(!game.Finished());
        CHECK(game.WinningLine().empty());
        CHECK(game.Turn() == caro::X);
    }

    // A gap-filling sixth stone must win; equality-to-five is incorrect.
    game.Reset();
    const int columns[] = {0, 1, 2, 4, 5};
    for (int k = 0; k < 5; ++k) {
        CHECK(game.Play(5, columns[k]));
        CHECK(game.Play(14, 2 * k));
    }
    CHECK(game.Play(5, 3));
    CHECK(game.Winner() == caro::X);
    CHECK(game.WinningLine().size() == 6);

    // O can win as well; both blocked ends still win under freestyle rules.
    game.Reset();
    for (int k = 0; k < 5; ++k) {
        CHECK(game.Play(14, 2 * k));
        CHECK(game.Play(0, k));
    }
    CHECK(game.Winner() == caro::O);
    game.Reset();
    const caro::Move moves[] = {{5, 1}, {5, 0}, {5, 2}, {5, 6},
                               {5, 3}, {14, 0}, {5, 4}, {14, 2}, {5, 5}};
    for (std::size_t i = 0; i < sizeof(moves) / sizeof(moves[0]); ++i)
        CHECK(game.Play(moves[i].row, moves[i].col));
    CHECK(game.Winner() == caro::X);
}

void Computer() {
    caro::Game game;
    CHECK(game.SuggestMove() == caro::Move(7, 7));
    // X has one available winning endpoint; O blocks the other endpoint.
    CHECK(game.Play(7, 1)); CHECK(game.Play(7, 0));
    for (int k = 2; k <= 4; ++k) {
        CHECK(game.Play(7, k)); CHECK(game.Play(0, 2 * k));
    }
    const std::string before = Snapshot(game);
    CHECK(game.SuggestMove() == caro::Move(7, 5));
    CHECK(Snapshot(game) == before); // hints never mutate a position
    CHECK(game.Play(7, 5));
    CHECK(game.SuggestMove() == caro::Move());

    // O must block X's only immediate winning endpoint.
    game.Reset();
    CHECK(game.Play(7, 1)); CHECK(game.Play(7, 0));
    CHECK(game.Play(7, 2)); CHECK(game.Play(0, 0));
    CHECK(game.Play(7, 3)); CHECK(game.Play(0, 2));
    CHECK(game.Play(7, 4));
    CHECK(game.SuggestMove() == caro::Move(7, 5));

    // Both sides threaten five: winning now is better than blocking.
    game.Reset();
    for (int k = 0; k < 4; ++k) {
        CHECK(game.Play(2, k)); CHECK(game.Play(8, k));
    }
    CHECK(game.SuggestMove() == caro::Move(2, 4));
}

void UndoTurns() {
    caro::Game game;
    CHECK(!game.UndoTurn(true));
    CHECK(game.Play(7, 7)); CHECK(game.Play(7, 8));
    CHECK(game.UndoTurn(true));
    CHECK(game.MoveCount() == 0 && game.Turn() == caro::X);
    CHECK(game.Play(7, 7));
    CHECK(game.UndoTurn(true));
    CHECK(game.MoveCount() == 0);
    CHECK(game.Play(7, 7)); CHECK(game.Play(7, 8));
    CHECK(game.UndoTurn(false));
    CHECK(game.MoveCount() == 1 && game.Turn() == caro::O);
    game.Reset();
    for (int k = 0; k < 4; ++k) {
        CHECK(game.Play(0, k)); CHECK(game.Play(14, 2 * k));
    }
    CHECK(game.Play(0, 4));
    CHECK(game.UndoTurn(true)); // X won before the computer could respond
    CHECK(game.MoveCount() == 8 && game.Turn() == caro::X && !game.Finished());
    game.Reset();
    for (int k = 0; k < 5; ++k) {
        CHECK(game.Play(14, 2 * k)); CHECK(game.Play(0, k));
    }
    CHECK(game.UndoTurn(true)); // O won: remove both O and the prior X move
    CHECK(game.MoveCount() == 8 && game.Turn() == caro::X && !game.Finished());
}

void Persistence() {
    caro::Game game;
    CHECK(game.Play(7, 7)); CHECK(game.Play(7, 8));
    const std::string saved = Snapshot(game, true);
    caro::Game restored(30);
    bool mode = false;
    std::istringstream input(saved);
    CHECK(restored.Load(input, mode));
    CHECK(mode);
    CHECK(Snapshot(restored, true) == saved);
    CHECK(restored.Undo());
    CHECK(restored.MoveCount() == 1);
    const char* invalid[] = {
        "", "CARO 2\n15 0 0\n", "CARO 1\n100 0 0\n",
        "CARO 1\n15 2 0\n", "CARO 1\n15 0 -1\n",
        "CARO 1\n15 0 226\n", "CARO 1\n15 0 1\n",
        "CARO 1\n15 0 1\n-1 0\n", "CARO 1\n15 0 1\n15 0\n",
        "CARO 1\n15 0 2\n0 0\n0 0\n", "CARO 1\n15 0 0\nextra",
        "CARO 1\n15 0 10\n0 0\n14 0\n0 1\n14 2\n0 2\n14 4\n0 3\n14 6\n0 4\n14 8\n"
    };
    for (std::size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        const std::string before = Snapshot(restored, mode);
        std::istringstream bad(invalid[i]);
        CHECK(!restored.Load(bad, mode));
        CHECK(Snapshot(restored, mode) == before);
    }
    std::istringstream valid("CARO 1\n5 0 0"); // no final newline is legal
    CHECK(restored.Load(valid, mode));
    CHECK(!mode && restored.Size() == 5);
}

void Geometry() {
    caro::BoardLayout layout(1000, 800, 15);
    caro::Move hit;
    CHECK(layout.cell > 0);
    CHECK(layout.Hit(layout.left, layout.top, hit));
    CHECK(hit == caro::Move(0, 0));
    CHECK(layout.Hit(layout.left + layout.cell, layout.top + layout.cell, hit));
    CHECK(hit == caro::Move(1, 1));
    CHECK(!layout.Hit(layout.left - 1, layout.top, hit));
    CHECK(!layout.Hit(layout.left, layout.top - 1, hit));
    CHECK(!layout.Hit(layout.left + layout.size * layout.cell, layout.top, hit));
    CHECK(!layout.Hit(layout.left, layout.top + layout.size * layout.cell, hit));
    CHECK(!caro::BoardLayout(20, 20, 15).Hit(0, 0, hit));
    CHECK(!caro::BoardLayout(100, 100, 0).Hit(0, 0, hit));
    for (int size = 5; size <= 30; ++size) {
        caro::BoardLayout fitted(1400, 1000, size);
        for (int r = 0; r < size; ++r)
            for (int c = 0; c < size; ++c) {
                CHECK(fitted.Hit(fitted.left + c * fitted.cell + fitted.cell / 2,
                                fitted.top + r * fitted.cell + fitted.cell / 2, hit));
                CHECK(hit == caro::Move(r, c));
            }
    }
}

// Independent brute-force winner oracle, used against seeded random games.
caro::Stone Oracle(const caro::Game& game) {
    const int dr[] = {0, 1, 1, 1}, dc[] = {1, 0, 1, -1};
    for (int r = 0; r < game.Size(); ++r)
        for (int c = 0; c < game.Size(); ++c)
            for (int d = 0; d < 4; ++d) {
                const caro::Stone player = game.At(r, c);
                if (player == caro::Empty) continue;
                bool line = true;
                for (int k = 0; k < 5; ++k)
                    if (!game.Inside(r + dr[d] * k, c + dc[d] * k) ||
                        game.At(r + dr[d] * k, c + dc[d] * k) != player) line = false;
                if (line) return player;
            }
    return caro::Empty;
}

void RandomGames() {
    std::mt19937 random(20260905);
    int draws = 0;
    for (int trial = 0; trial < 120; ++trial) {
        caro::Game game(trial < 100 ? 5 : 15);
        while (!game.Finished()) {
            caro::Move candidate;
            if (trial >= 110) candidate = game.SuggestMove();
            else {
                do {
                    candidate = caro::Move(static_cast<int>(random() % game.Size()),
                                           static_cast<int>(random() % game.Size()));
                } while (game.At(candidate.row, candidate.col) != caro::Empty);
            }
            CHECK(game.Play(candidate.row, candidate.col));
            CHECK(game.Winner() == Oracle(game));
        }
        if (game.Draw()) {
            ++draws;
            CHECK(game.MoveCount() == game.Size() * game.Size());
            CHECK(game.SuggestMove() == caro::Move());
        }
        bool mode = false;
        std::istringstream saved(Snapshot(game));
        caro::Game loaded;
        CHECK(loaded.Load(saved, mode));
        CHECK(loaded.Winner() == game.Winner());
        CHECK(loaded.Draw() == game.Draw());
        while (game.MoveCount() > 0) {
            CHECK(game.Undo());
            CHECK(!game.Finished());
            CHECK(game.Winner() == Oracle(game));
        }
        CHECK(game.Turn() == caro::X);
    }
    CHECK(draws > 0); // exercises full-board draws, not just wins
}
} // namespace

int main() {
    try {
        Rules(); Computer(); UndoTurns(); Persistence(); Geometry(); RandomGames();
        std::cout << "PASS: " << checks << " checks (rules, AI, saves, geometry, 120 seeded games)\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}

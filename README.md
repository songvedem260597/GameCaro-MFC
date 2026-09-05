# Game Caro MFC

A native Windows Caro/Gomoku game in C++ and MFC. Open **`vebanco.sln`** for the
main application. `Threading/` is a separate historical sample and is unchanged.

## Gameplay upgrade

- Human vs computer (default: human X, computer O), or two local players.
- 15x15, 20x20 and 30x30 boards; switch using the new **Caro** menu.
- Freestyle rules: **five or more consecutive stones** win in any of four axes.
  Four stones do not win. Blocked endpoints do not invalidate a win. This is
  not Renju, exact-five, or the blocked-both-ends Caro variant.
- A full board without a winner is a draw. No moves are allowed after a result.
- Undo one move in two-player mode, or the human/computer pair in computer mode.
- Tactical hints; highlighted last move, suggested cell and winning line.
- Responsive board layout, half-open click boundaries, double-buffered drawing.
  Repainting, covering or resizing the window no longer removes placed stones.
- Versioned text saves with input validation; invalid saves are never partially
  applied to the game model. New game / mode / board changes prompt to save
  modified games before discarding them.

The computer is a deterministic **one-ply heuristic**, not minimax or a trained
model. It prioritizes an immediate win, blocks an immediate loss, then scores
open attacking and defensive lines. Multiple simultaneous threats can beat it.

## Controls

| Action | Control |
| --- | --- |
| Place a stone | Left-click an empty cell |
| New game | F2, Ctrl+N, or Caro > Van moi |
| Undo | Ctrl+Z or Caro > Hoan tac |
| Show a suggested move | H or Caro > Goi y |
| Change mode / board size | Caro menu (starts a new game) |
| Save / open a game | Ctrl+S / Ctrl+O, or the File menu |

Give saves a `.caro` filename. The original File dialog resources are retained;
select **All files** when necessary. The format begins with `CARO 1`, followed
by size, computer-mode flag, move count, then zero-based row/column pairs.
Loading an unfinished computer-mode game with O to move lets the computer play.
Empty files produced by the old, unimplemented serializer are not valid saves.
UI labels use unaccented Vietnamese to remain compatible with legacy MSVC
source encodings.

## Build the Windows application

Install Visual Studio with **Desktop development with C++**, the **MFC** component
matching your chosen toolset, and a Windows SDK. Open `vebanco.sln`, choose
**Win32**, and build/run `vebanco`.

The checked-in project is historical: Debug targets v140, Release targets
v110, and the Windows SDK property is 8.1. These settings are intentionally not
silently retargeted. On a modern Visual Studio installation, retarget the SDK
and select the same installed platform toolset for **both** configurations in
Project Properties. For example, from a VS 2022 Developer Command Prompt with
v143, matching MFC and a Windows 10/11 SDK installed:

```bat
msbuild vebanco.sln /m /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143 /p:WindowsTargetPlatformVersion=10.0
```

`CaroGame.h` is header-only and is included by the document, so no additional
link dependency or project source entry is required. The old `oco` helper is
retained with safe bounds, five-or-more checks, initialized coordinates and no
global `max` macro. The active view uses the tested `caro::Game` model instead.

**Rebuild the application.** Historical EXE/PDB/IDE files already tracked by the
repository are not regenerated or deleted by this change. They do not contain
the new features. `.gitignore` prevents new build/IDE output from being added;
it does not remove files that Git already tracks.

Microsoft documentation:
- https://learn.microsoft.com/en-us/cpp/mfc/reference/creating-an-mfc-application
- https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class
- https://learn.microsoft.com/en-us/cpp/mfc/reference/carchiveexception-class

## Portable regression tests

CMake at the repository root builds **only the portable tests**, not the MFC
application. A C++11 compiler is sufficient; no third-party libraries needed.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Checks cover all four win directions, board edges, four-not-five, overlines,
both players, blocked endpoints, draws, rejected moves, undo after results,
computer-mode undo, tactical wins/blocks, non-mutating hints, save round trips,
malformed/unsupported/truncated saves, geometry and 120 seeded games checked
against an independent winner oracle. `CHECK` remains active in Release builds.

Validated for this change on Linux:
- GCC and Clang, C++11, strict warnings as errors: **48,890 checks pass** each.
- GCC AddressSanitizer + UndefinedBehaviorSanitizer: **48,890 checks pass**.

**Not yet verified:** a real Windows/MSVC/MFC build and interactive UI behavior.
Passing portable tests does not establish that the Windows application builds.

### Windows smoke-test checklist

1. Build both Win32 Debug and Release with matching SDK/toolset/MFC components.
2. Open the Caro menu, switch modes and all three board sizes.
3. Place moves, cover/uncover, minimize/restore and resize; stones must persist.
4. Click exact grid lines and outside all board edges; one click places at most
   one human stone (the computer may reply with its own stone).
5. Check wins, the highlighted winning line and rejection of post-result moves.
6. Undo during play and after X/O wins; computer mode must return to X's turn.
7. Save/reopen both modes, reject malformed/truncated saves, and cancel the
   save prompt when switching mode or starting a new game.
8. Check F2, H, Ctrl+Z, Ctrl+N/O/S and print preview; observe GDI handles over
   repeated redraws to verify objects are not accumulating.

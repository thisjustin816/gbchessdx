# Independent chess rules host candidate (2026-11-03)

Status: **experimental, unintegrated**, not a new promoted checkpoint. The last promoted research checkpoint remains v1.2.1 (source ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`). The v1.2.1 source bytes were not available for modification in this session. No changes to `master` or pinned MIT `engine.c`.

## Recoverable add-on

- Archive: `GB-Chess-Independent-Rules-EXPERIMENTAL-2026-11-03.zip`
- SHA-256: `b86d8fc22b1dc2cc3c2190ed7612b735dcddd8a66a206e61e9214dae2e6cc432`
- Standalone `STATUS-HANDOFF.md` SHA-256: `0d00e9c9e2d64b2423d084b9e1f6be0072c345136d28e2d4efcfd7ca6166da16`
- The archive contains Python source, tests, README, status handoff and a five-record file hash manifest. Fresh extraction and deterministic byte-identical repack passed. Files are in the ChatGPT GB Development conversation, not stored as a ROM or proprietary asset.

## Tested scope

Dependency-free host rules candidate: legal king safety, standard castling and rook rights, en passant including discovered checks, four promotion choices, reversible moves, FEN, threefold/fivefold repetition, 50/75-move thresholds and conservative known dead positions.

**20 canonical perft checks passed** against fixed independent reference counts from chessprogramming.org (opening through depth 4, Kiwipete through depth 3, positions 3-6 through depths 3 or 4). Special-move/draw-state tests, two negative rights/EP mutations and 20 deterministic legal random walks (1,300 plies) passed. Undo restored exact FEN, castling rights, EP and counters at each ply.

## Limits and next steps

This is a host test candidate, not a Game Boy rules core or complete FIDE oracle. Remaining: complete dead-position adjudication, prospective draw claims, external Stockfish/python-chess differential play, more edge-case tactics and a compact cartridge implementation. The current AI remains unselected. Revalidate v1.2.1 raw source before shared integration; keep the board-first lesson layout and five planned modes unchanged.

Project card: https://trello.com/c/FYELcNLg

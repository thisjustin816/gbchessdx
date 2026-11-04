# Chess Academy — research checkpoint v1.0.0

Research/evidence only. No playable Story Mode ROM, selected AI engine, finished boss matches, new user-facing game UI or change to master.
Trello: https://trello.com/c/FYELcNLg

## Recoverable checkpoint
- GB-Chess-Research-v1.0.0-source-and-evidence.zip — 846,080 bytes, 505 ZIP entries, 504 per-file SHA-256 records.
- Archive SHA-256: 106f03303de86811e4057b939896cefa097de7bf4a1c35b3ee5e5fb48291cc43
- GB-Chess-STATUS-HANDOFF-v1.0.0.md SHA-256: 4b04c3bb4e974886a582f044a52a9f70381ffa24a6dfb63da5aaca1709701e22
- Clean extraction validated all 504 recorded file hashes, full 16-group offline regression suite, and byte-identical standalone handoff. Deterministic repack reproduced the same ZIP hash. Complete source and evidence are downloadable from the current GB Development ChatGPT conversation.
- Fresh GBDK compilation (not cached) from the clean extraction reproduced terminal-case diagnostic ROM SHA-256 5091cd12d334848e75b514d2ef66f5b3f3f440e254f5ff6fbe596cc3ff0e8867 and no-legal-move results on two cold boots.
- Source archive contains no ROMs, saves, emulator state, toolchains, compiled objects or nested ZIPs. Original proprietary Chessmaster reference ROM remains external and unmodified. ncw/gbchess MIT upstream engine.c Git blob 495890f379d449be5c051f8525034881574716ff unchanged.
- Previous v0.9 checkpoint preserved, SHA-256 efcef799cd29745520774cbb8c242b2839074bb52619dc0c8397e76ad267f390.

## Shallow explicit-state chess search candidate
A separate source-linked two-ply candidate runs root/reply/finalize stages in foreground frame slices, restoring board state after each step. No modification to the original recursive computer_move implementation. The legal-move fallback, cancellation and terminal no-move cases are guarded. This is not a selected strong AI or general resumable alpha-beta engine.
- 13 configurations x 2 matching cold boots = 26 PyBoy 2.8.1 result JSONs. Dual-mode 32 KiB GBDK, CGB double-speed and original DMG fallback verified, header $0143=$80, KEY1, exact ROM hashes, position hashes, independent sparse-position oracle, mutation tests.
- Full knight fork: GBC 261 frames / 261 work units, DMG 267 frames / 261 work units, move g5e4 (MISSES desired fork g5f3). GBC short B cancellation happened to return g5f3 at 45 frames; DMG 44 frames. A short A tap retained.
- Unique mate: queen from d4 to f2 (d4f2) correctly on both consoles in 329 foreground passes. Promotion e7e8q on both in 55 passes. Checked-king cancellation returns legal e1d1. Checkmated king has no legal result and skips search.
- Maximum observed input sampling gap one display frame on tested GBC cases; two frames in DMG fork. Individual steps still have variable execution cost and the synchronous legal-fallback preflight can be expensive on other boards. No physical hardware, full midgame workload or end-to-end UI guarantees.
- Diagnostic status regression: old checkmate status was correctly no-move but the internal final-state field stayed 0; new 4 matches, old-fails/new-passes evidence retained.

## Story campaign and historical review work
- Three-ply fork boss miniature now accepts all four legal defender king replies. Outcome derived by independently replaying actual legal moves and inspecting the captured queen; wrong, incomplete or tampered move histories rejected. A deterministic legal default defender remains a prototype, not a selected chess engine or true full match.
- analysis/HISTORICAL-CHESS-GAME-REVIEWS-v1.0.md cites 21 external source links (critics, retrospectives, labeled low-confidence anecdotes). Historic praise: readable 2D boards, adjustable opponents, legal-move help and undo, staged instruction, thematic quest progression and personality. Historic criticisms: unclear hard exercises, clunky menus, repetitive nontransfer mini-games and animations that delay actual chess.
- Important lesson for authored challenges: accept equivalent legal answers and valid alternative defender moves, rather than requiring a single memorized script. Distinct boss style needs measured chess decisions, not only portraits and names. Final story setting, chapter roster, art and save design remain open.
- Preserve the approved board-first six-screen lesson storyboard and dual-mode GB/GBC compatibility.

## Next
Stress full starting and midgame boards for worst-case per-step latency, memory/stack and cancellation; independently verify full-FIDE rules and tactic quality against external references; plan meaningful authored lesson and boss mini-match content; seek mockup approval before user-facing UI; validate physical DMG/GBC/Chromatic. The original gbchess engine remains a candidate, not selected.
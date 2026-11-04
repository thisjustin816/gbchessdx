# GB Chess Academy — research checkpoint v1.3.0

Research-only checkpoint. **No playable game, production AI, final screens, save or link implementation.** `master` and the pinned MIT `ncw/gbchess` upstream engine remain unchanged. Complete source and evidence are delivered in the GB Development ChatGPT project conversation rather than committed as individual C files. Project planning: https://trello.com/c/FYELcNLg

## Exact recoverable source

- `GB-Chess-Research-v1.3.0-source-and-evidence.zip` — **956,675 bytes**, **613 entries**, **612 per-file SHA-256s**.
- ZIP SHA-256: `e604a07e57f9e63a51b59f3694c8bf8d19b3ae696087b35768cf64fee1f65cf0`.
- Standalone `GB-Chess-STATUS-HANDOFF-v1.3.0.md` — **48,357 bytes**, SHA-256 `b9c5f3dd530b9b09207a4ed9b209f91078c0a6ffc8331f811abfae2c1ff5057f`. Byte-identical to the handoff inside ZIP.
- Prior verified v1.2.1 is retained separately under ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`; separately published v1.2.0 under `54a79fcf554c845b27ebda637a67df9432b0b77ac6f256d2bcf9a9e685967db0` remains immutable.
- Clean extraction verified all 612 file hashes, all **19 offline verifier groups**, deterministic repackaging with identical ZIP bytes, and zero prohibited ROM/save/object/emulator-state/toolchain/nested ZIP entries.
- Uncached warning-clean GBDK build from the recovered source reproduced Kiwipete dual-mode diagnostic ROM SHA-256 `1382ef6dc2e6a396da07cfa385ce2f679c4268855b80acc7d6d3938eda480649`, static WRAM end `0xC256`, and two bit-identical cold boots on each of CGB double-speed and original DMG. No generated ROM is in the source ZIP.

## Independent chess rules, old fails / new passes

- New independent portable C `analysis/fide-rules/rules_core.{c,h}`, `rules_status.{c,h}` (original project code, **not** the upstream AI or proprietary Chessmaster). Q/R/B/N promotion, king-safe castles with rook presence, legal en passant, clocks, position identity, compact reversible make/unmake and incremental legal move generation.
- Pinned upstream `gbchess` engine reproduces **five mismatches** against 18 canonical perft counts (six positions × depths 1–3), concentrated on missing underpromotions. In particular, position 4 d3 **8,087 vs 9,467**; position 5 d3 **54,007 vs 62,379**.
- New core matches **all 18 independently published totals**, with 16 additional special-case assertions, 12 exact incremental/full move-set comparisons and byte-exact compact undo for each legal move in eight positions. It handles checkmate, stalemate, several dead positions, post-move claimable 50-move and threefold, automatic 75-move and fivefold results. Full move-history persistence, prospective draw claims and exhaustive dead-position classification remain open.
- Host suite re-compiles original upstream negative baseline and new rules core with `-Wall -Wextra -Werror` and UBSan. An intentional underpromotion mutation is rejected. Original upstream `engine.c` Git blob `495890f379d449be5c051f8525034881574716ff` unchanged.
- Published reference tests: FIDE Laws (https://handbook.fide.com/chapter/e012023) and canonical perft FEN/count set (https://github.com/Wasserpuncher/chess-perft).

## Incremental move generation on Game Boy

Five chess-specific diagnostic cases × original DMG/GBC × two cold boots = **20 recorded PyBoy 2.8.1 JSONs**, each with pinned ROM hash and independent fixed expectations. Five evidence mutations rejected. 32 KiB dual-mode ROM, CGB flag `$80`, checked header/global checksum and KEY1, CGB double-speed; no UI/audio/save/link code.

| Position | Legal moves | GBC max step scanlines | DMG max step scanlines | GBC/DMG total frames |
| --- | ---: | ---: | ---: | ---: |
| Start | 20 | 22 | 45 | 149 / 149 |
| Kiwipete | 48 | 71 | **143** | 177 / 178 |
| Promotion/check | 44 | 48 | 97 | 173 / 173 |
| Castling/promotion | 6 | 24 | 49 | 167 / 167 |
| Open queen | 29 | 59 | 118 | 158 / 158 |

Four promotion choices verified in *both* console modes. Kiwipete DMG observed two-frame input sampling gap. **143 of 154 scanlines** on a DMG step is too close to the display budget for a complete interface. One generator step per frame takes 149–178 frames merely to enumerate legal moves. This is an experimental foundation for bounded work, **NOT a strong resumable AI**, production response guarantee or physical hardware evidence.

## Next development frontier

Build a measured scheduler that batches safe work but yields during expensive move generation and evaluation, then compare a genuinely stronger AI on independently checked tactics. Expand legal game-state history, prospective threefold/50-move claims, complete FIDE dead-position handling, undo/save tests, full UI/audio memory and real DMG/GBC/Chromatic verification. Keep the approved 160×144 board-first lesson storyboard and the five requested future modes (Campaign boss matches, Coaching hints/evaluation/takebacks, Puzzles, selectable VS Computer, Pass & Play/Link Cable). No engine or narrative setting is final.

# GB Chess Academy research v0.10.0: historical reviews and five game modes

**Checkpoint type:** design/source/evidence only. No playable game ROM, selected engine, new UI implementation or change to `master`. Trello: https://trello.com/c/FYELcNLg

## Exact checkpoint / recovery

- Archive: `GB-Chess-Research-v0.10.0-source-and-evidence.zip`, **825,598 bytes**, **472 entries**, 471 per-file SHA-256 records.
- ZIP SHA-256: `c68289d254a73f540dd4e39cae6838f4e740471d4ebff6f20903bdb7aae9b188`.
- Standalone handoff: `GB-Chess-STATUS-HANDOFF-v0.10.0.md`, SHA-256 `7bbc1c5c616a83e284e31ed7e1bc415eb1248f976d64c680c3a32eff43ab919f`.
- Historical reviews: `GB-Chess-Historical-Game-Reviews-v0.10.0.md`, SHA-256 `4dd25200070b3448948c633baba8588836a6ef44e8501e1124bb207161e4e118`.
- Mode specification: `GB-Chess-Game-Modes-v0.10.0.md`, SHA-256 `9edc32f938d978148ee1706de6a5dabbc13696dc0a4986226d45c35b3ad98f64`.
- Clean extraction PASS, 471/471 file hashes PASS, all **15** offline verification groups PASS, 6 intentionally mutated mode contracts rejected, deterministic repackaging matches archive SHA. v0.9 handoff byte-identical to frozen predecessor. No ROMs, save files, builds or toolchains included. All four files downloadable from the GB Development ChatGPT conversation.

## User-directed game structure

Five separate top-level modes:
1. **Campaign**: story chapters with lessons → varied practice → actual chess boss matches, not repeated single-position puzzles.
2. **Coaching**: play full chess with opt-in staged hints, move feedback, review and full-turn takebacks.
3. **Puzzles**: independently verified standalone tactical/endgame problems, untimed by default; retries, themes, explanations.
4. **VS Computer**: standard chess with user-selected difficulty and no forced training or unsolicited feedback.
5. **2 Players**: **Pass & Play** on one handheld and **Link Cable** on two, shared FIDE rules and mutual-consent takeback policy.

The five mode *requirements* are user-decided. Specific UX/algorithms and scope ordering remain proposals. All modes currently DESIGN ONLY; campaign and input-pump are host/test fixtures, not a playable game.

## Historical review study

The clearest predecessor is **Hoyle Majestic Chess (2003)**: critics liked story quests, chess lessons, transfer tests, and chapter-closing opponent challenges, but a minority criticized novice teaching and limited advanced analysis.

Other repeated signals:
- **The Chessmaster GB (1991), Xbox (2004), Chessmaster 10th Edition:** advice, legal destinations, undo/replay, learner-friendly difficulties and distinct AI personalities are valued; excessively nested menus, weak explanations, artificial cosmetic locks and controller quirks are criticized.
- **Battle/Combat Chess**: personality and capture animations make chess inviting, but repeated lengthy animations lose appeal.
- **LEGO Chess / Story Time Chess**: character-driven reminders help teach movement but need transfer to ordinary chess.
- **Chessaria and Chessarama**: novel story and puzzle ideas work, while repetition, difficulty spikes and interface friction are pitfalls.
- **Kasparov Chessmate and Chess Ultra**: human multiplayer options matter, but controls and connection reliability need care.
- **Chess.com, Dr. Wolf, Lichess Puzzle Streak**: actionable feedback/retry and optional low-pressure puzzle progression are good reference points, not cartridge-ready requirements.

Detailed review report includes URLs, dated source attributions and limits; it is a qualitative non-representative source scan. Representative primary sources:
- https://www.gamespot.com/reviews/hoyle-majestic-chess-review/1900-6074888/
- https://www.gamespot.com/articles/hoyle-majestic-chess-impressions/1100-6025714/
- https://www.nintendolife.com/reviews/2010/09/chessmaster_retro
- https://www.gamespot.com/reviews/chessmaster-review/1900-6112556/
- https://www.gamespot.com/reviews/chessmaster-10th-edition-review/1900-6106812/
- https://www.nintendolife.com/reviews/switch-eshop/chess_ultra
- https://www.metacritic.com/game/chessarama/critic-reviews/
- https://www.reddit.com/r/chess/comments/mfngdo/lichess_new_feature_puzzle_streak/

## Technical boundaries / next tests

All modes require a shared full FIDE chess-state implementation, saved complete move history with castling/en-passant/repetition/counters, stable FEN/lesson IDs, legal promotion and terminal states. The v0.9 interrupt-latch test is not a resumable search; coach evaluation must be confidently attributed or explicitly unavailable. Boss victory must come from a genuine game result, not an arbitrary caller flag. Link Cable needs independently stepped protocol and physical DMG/GBC/Chromatic fault tests, not just PyBoy. Continue paired modern gbchess and private Chessmaster tests; no engine or numeric Elo selected.

The existing board-first lesson storyboard, dual-mode GBC-double-speed/DMG-fallback target, and upstream MIT engine identity remain frozen.
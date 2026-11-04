# Engine research — v0.4.0 (2026-11-03)

**State:** research only; **no player game ROM, engine choice, or source changes to upstream**. The proposed original GB/GBC game teaches chess through a full-board-first UI with separate hint and explanation pages. The user approved the overall six-screen storyboard, not its final font/pieces or UI move-highlight styling.

**Planning log:** https://trello.com/c/FYELcNLg
**Development fork:** https://github.com/thisjustin816/gbchessdx (branch `research/engine-benchmark-v0.4`; leave `master` untouched until tests and source checkpoint are promoted).
**Note:** GitHub Issues are disabled on this repo (API 410); keep progress on Trello and this branch.

## Recoverable research checkpoint

`GB-Chess-Research-v0.4.0-source-and-evidence.zip`, **641,024 bytes**, SHA-256 `4bf06adccda794192a9a726164f13a40895e1307e5b371833c6146deb3da95ae`, supplied as a downloadable artifact in the project ChatGPT conversation. 326 files including `MANIFEST.json`, `STATUS-HANDOFF.md`, pinned MIT upstream source, scripts, original 160×144 mockups, and repeatable benchmark evidence. **No game ROMs, commercial ROMs, saves, compiled objects, or nested ZIPs.** Clean-extraction hash validation and all five offline verification suites passed. From the extracted ZIP, a GBDK/PyBoy fixture rebuilt to the exact pinned ROM SHA and repeated its move/time evidence.

Reference ROM SHA-256 (held separately; **do not commit/upload**):
- `The Chessmaster (USA) (Rev 1).gb`: `0a44d1f127436d73fc2f64201da0e0fd40793a414e533bba0a08e02dfb51cb10`. Proprietary; used only for behavioral comparisons.
- `gbchess.gb`: `3b234fabf81d5155037c66197006c335fe19106f71659e2bcd22829f2a38b92f`. Bundled ROM from supplied `gbchess-master.zip` is byte-identical. Source archive SHA `548eba25a85df5b091afcb0729b7b7c1826c4ce7cdcf53738ffcae41eeb0a889`.

## Verified engine comparisons

- Original MIT-licensed `ncw/gbchess` source remains unmodified; fork's README, engine.c and Makefile Git blob SHAs matched upstream on 2026-11-03; engine.c SHA `495890f379d449be5c051f8525034881574716ff`.
- Host source builds and standard opening perft depths 1–5 pass: 20, 400, 8,902, 197,281, 4,865,609 nodes.
- A separate FEN host adapter calls upstream `search_best_move(fixed_depth)`, avoiding opening-book, random beginner choices and sparse-position depth boosts. Against seven Chessmaster reference tactics, target moves pass **4/7 at depth 1** and **7/7 at each depth 2–4**.
- GBDK test-only fixtures use **original DMG** header flag 0x00 and PyBoy 2.8.1 with `cgb=False`, measuring `bench_status` search start/end in WRAM. All four cold-boot repetitions match:
  - Knight fork `4k3/8/8/6n1/8/8/3Q4/4K3 b - - 0 1`: d2 `g5f3` 86 frames (~1.44s); d3 `g5f3` 174 frames (~2.91s).
  - Unique mate `4k3/8/8/2b5/3q2b1/3p4/8/4K3 b - - 0 1`: d2 `d4f2` 495 frames (~8.29s); d3 `d4f2` 1,993 frames (~33.37s).
- One test harness error was diagnosed and corrected: the initial three fixtures were compiled GBC-only via `-Wm-yC`. They were **not valid original-DMG evidence**. Rebuilt without the flag, explicitly forced DMG and reran both cold-boot repetitions; moves and timings matched. Misconfigured results are labeled historical in the checkpoint.
- These are **emulated search-only** times, not physical hardware or full UI move latency. Not a Chessmaster-vs-gbchess strength/Elo ranking.

## Development choices and next tasks

We currently **favor continued MIT gbchess evaluation over a proprietary Chessmaster ROM hack**, without selecting the engine yet. Preserve its MIT license and attribution.

1. Expand tactics/endgames using a general chess legality oracle, including defensive/noncapture choices and more than one valid solution.
2. Evaluate actual `computer_move()` game-level timing and practical interactive hint budgets; authored explanation with bounded search likely preferable to 33-second depth-3 hints.
3. Prototype a native 160×144 board-first lesson on a new branch *after* verifying ROM/RAM, save/SRAM, GBC enhancements, link cable and font/piece artwork budgets.
4. Keep Trello updated after meaningful changes and preserve exact source+ROM+handoff hashes for every promoted build. Commercial Chessmaster ROM/code remains outside the repo.

For complete commands, read `modern-engine-research/README.md` in the v0.4.0 source ZIP.
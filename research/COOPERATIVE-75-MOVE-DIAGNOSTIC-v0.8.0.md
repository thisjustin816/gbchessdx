# Automatic 75-move cooperative search diagnostic v0.8.0

**VERIFIED standalone experimental source checkpoint** (2026-11-04 Pacific). Not a playable chess game, selected engine or integration into the promoted main project. This note supersedes the prior **unverified v0.8 WIP** listed in the Codex transfer card; that WIP ZIP remains historical. The promoted main GB Chess Academy research checkpoint remains **v1.3.0**, source ZIP SHA-256 `e604a07e57f9e63a51b59f3694c8bf8d19b3ae696087b35768cf64fee1f65cf0`; its exact source bytes were not available for this integration. `master` and pinned upstream MIT `engine.c` untouched.

## Exact source and build recovery

- `GB-Chess-Search-EXPERIMENTAL-v0.8.0-source-and-evidence.zip` — **146,665 bytes**, 77 entries, 76 individually verified source/evidence SHA-256 records. ZIP SHA-256 `2dd1940c4a9218a44fef7fed0b600aff80a4c0306560add12de5bceef4c749ae`.
- Standalone `GB-Chess-Search-EXPERIMENTAL-v0.8.0-STATUS-HANDOFF.md` — 8,803 bytes; SHA-256 `ec4b4cbf1b81b25b62e65934a798a2d7dfadbc0ffbf28ee15bd4ec44bd4fceaf`, byte-identical to archived `STATUS-HANDOFF.md`.
- Optional nonplayable dual-mode `GB-Chess-Search-EXPERIMENTAL-v0.8.0-diagnostic.gb` — 32,768 bytes; SHA-256 `5ad0664c7c9559e7828e76b9cc3c114ec76db939e9d36403b45ed91bd9df1269`, MD5 `001c2e76b2fd8311baed5181ad7ee06c`; **not inside source ZIP** and must be distributed only alongside its matching source/handoff.
- All three exact artifacts are available as downloads from the current GB Development ChatGPT conversation.
- Independent clean extraction: 76/76 per-file sizes/hashes pass; no forbidden binaries/saves/toolchains/objects/nested archives; deterministic repack byte-identical to ZIP; standalone handoff byte-identical.
- Clean-extracted source: **17/17 offline verification groups pass**, including inherited 15 v0.7 groups, new fixed 75-move host regression and the 12-record frozen-evidence verifier rejecting eight mutations. Targeted host UBSan passed on quiet threshold, pawn reset and checkmate exception. Original v0.7 source is preserved under `historical/search-v0.7.c`.
- Uncached GBDK rebuild from **that clean extraction** reproduced ROM SHA/MD5 exactly, 32 KiB $80 CGB/DMG dual-mode cartridge, ROM-only, header/global checksums valid, CGB KEY1 fast and original DMG fallback. Static linker WRAM end **$C3E9**; not a dynamic stack certificate.
- **12 independent PyBoy 2.8.1 cold boots** (CGB/DMG x none/A/B two-frame pulses x two) passed, with full regenerated JSON byte-identical to archived `evidence/75-coldboots-12.json`. Each of 11 positions preserved board integrity and the expected moves/scores. Maximum sampled input polling gap CGB **2** / DMG **3** frames, step crossings **1/2**. These are emulator position-specific samples, not physical hardware or universal real-time bounds.

## Old-fails/new-passes rules

Frozen v0.7 mistakenly values the quiet rook move at halfmove 149 as +500, although it advances to 150 halfmoves and invokes the automatic 75-move draw. v0.8 values it as **0**, retains +500 one halfmove earlier, chooses `a2a3` +600 when a pawn can reset the counter, and avoids treating 50 moves as automatically drawn. Unique `d4f2` mate at halfmove 149 remains +29999 because checkmate on the final move takes precedence. Captures also reset the counter. The separate interior-node early draw cutoff is mutation-tested for saving work, and cancellation still safely unwinds the board.

The previously failing `verify_seventyfive_evidence.py` assertion incorrectly assumed identical completed-depth arrays for CGB and DMG after B cancellation. Frozen data proves **CGB depth 1 (104 steps)** and **DMG depth 0 (30 steps)**; both choose the legal fallback. Pinning those exact per-mode values makes the 12-record verifier pass and still reject eight evidence mutations. No ROM code was changed to fix the verifier.

## Remaining frontier

This search is a material-only cooperative experiment, not complete FIDE adjudication: missing prospectively claimable draws, full repetition history, fivefold, dead-position handling and game-level root adjudication. A stronger chosen AI, realistic UI/audio timing, physical DMG/GBC/Chromatic, save/link and player-facing five modes remain undone. Recover/compare main v1.3.0 source only if available through authorized project files; do not request missing files from the owner. No integration or version promotion to the main game. Trello stays Exploring: https://trello.com/c/FYELcNLg

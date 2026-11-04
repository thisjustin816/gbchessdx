# Stalemate-aware cooperative search diagnostic v0.7.0

Status: verified standalone experiment, not the promoted gbchessdx source, a selected chess AI, or a playable game. Main research baseline remains v1.2.1 (archive SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`); its exact source was not recovered for integration. Default `master` and pinned upstream MIT `engine.c` remain unchanged.

## Recoverable checkpoint

- Source and evidence: `GB-Chess-Search-EXPERIMENTAL-v0.7.0-source-and-evidence.zip`, 122,221 bytes, 66 entries and 65 file hash records; SHA-256 `f4ed65c3373a4c8ca70f2a2f3480f9013905364f16255abc3b2dc741ac0e0208`.
- Standalone `GB-Chess-Search-EXPERIMENTAL-v0.7.0-STATUS-HANDOFF.md`, SHA-256 `2e26d621f5ad54813628eb36e56b661422dfc36bb065cd51642e4af204f1ae6e`, byte-identical to the archived handoff.
- Optional nonplayable dual-mode `GB-Chess-Search-EXPERIMENTAL-v0.7.0-diagnostic.gb`, 32,768 bytes, SHA-256 `3d771ecc6f04934eeacb01b6e7080d3c09ee5092e174b7e7fa37e6675ebec126`, MD5 `20047f7e8aaa8fa6465bf0df79b11fd1`. The ROM is outside the source ZIP and must be paired with its exact source and handoff.
- All three artifacts are available in the GB Development ChatGPT conversation. A fresh extraction matched every archived size/hash, excluded ROMs, tools and build objects, and repacked to the same ZIP byte-for-byte. GBDK rebuilt the exact ROM SHA/MD5, checked the dual-mode $80 header, checksum, DMG fallback and CGB double-speed, and reported linker static WRAM end $C3A3.

## Reproduction and results

At FEN `k7/8/1Q1K4/8/8/8/8/8 w - - 0 1`, frozen v0.6.0 depth-one search chose `b6c7` (+900), incorrectly overlooking that it stalemates Black. New source gives stalemate score 0 and chooses `b6d8` (+900), preserving legal Black replies. The prior v0.6.0 source and old-fails/new-passes test are retained. Depth-one mate solutions `d4f2` and `g6g7`, both +29999, remain correct. Cancellation during an active nonchecked leaf still restores the working board and keeps a legal fallback.

An optional positive-only pawn-reply witness avoids full reply scans only when an unobstructed pawn push cannot uncover a king attack. Its positives matched an independent Python legal-move oracle across 1,055 fixed-seed positions (1,014 positive cases, no false positives). Failure to find this witness is inconclusive and falls back to the cooperative cursor.

All **15 offline groups** passed after clean extraction: inherited rules, 20 published perft counts, 1,988 C/Python legal-position comparisons, 22 independent Python minimax positions, original evidence and mutation controls, new stalemate/cancellation tests, pawn-witness comparisons, and eight rejected new JSON mutations. Targeted host UBSan runs passed.

The exact clean-built ROM passed **12 independent PyBoy 2.8.1 cold boots** (CGB/DMG x none/A/B x two); complete regenerated JSON is byte-identical to frozen evidence. Six scenarios cover both mates, depth-two mate, opening watchdog/B cancellation, checkmated terminal, and the stalemate trap. Two-frame A and B inputs are detected. Search batches are capped at six CGB or three DMG work units, breaking after a step crosses VBlank.

Sampled elapsed CGB/DMG frames: depth-two six-piece mate 339/698; stalemate trap 325/641. Maximum sampled foreground input-poll gaps 2/3 frames, and individual search-step crossings 1/2 frames. Eight-unit CGB batching was rejected for a three-frame poll gap, while four units cost more elapsed time; these alternative measurements are preserved. The observations are emulator-specific, not hardware or worst-case timing bounds.

## Limits

Final-ply stalemate is fixed, but full FIDE draw claims, repetition history, 50-/75-move rules, dead-position adjudication, stronger evaluation, realistic UI/audio timing, physical DMG/GBC/Chromatic tests, SRAM and link play remain unverified or unimplemented. The approved board-first lesson storyboard and five planned modes remain unchanged. Do not integrate into the promoted v1.2.1 tree without recovering its exact source. The project remains Exploring.

Trello: https://trello.com/c/FYELcNLg

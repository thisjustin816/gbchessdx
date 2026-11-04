# Stalemate-aware cooperative search diagnostic v0.7.0

Status: verified standalone experiment, not promoted main gbchessdx source, a selected AI or a playable game. Main research **v1.3.0** was subsequently promoted, separately from this experiment: `GB-Chess-Research-v1.3.0-source-and-evidence.zip`, SHA-256 `e604a07e57f9e63a51b59f3694c8bf8d19b3ae696087b35768cf64fee1f65cf0`. Its matching handoff SHA-256 is `b9c5f3dd530b9b09207a4ed9b209f91078c0a6ffc8331f811abfae2c1ff5057f`. The previous main v1.2.1 archive (SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`) remains preserved. The v1.3.0 raw source was not locally available for integration in this experiment; no merge or comparison of the two rules candidates has been performed. `master` and pinned upstream MIT `engine.c` are unchanged.

## Recoverable checkpoint

- Source/evidence: `GB-Chess-Search-EXPERIMENTAL-v0.7.0-source-and-evidence.zip`, 122,221 bytes, 66 entries, 65 manifest SHA-256 records; SHA-256 `f4ed65c3373a4c8ca70f2a2f3480f9013905364f16255abc3b2dc741ac0e0208`.
- Matching standalone `GB-Chess-Search-EXPERIMENTAL-v0.7.0-STATUS-HANDOFF.md`, SHA-256 `2e26d621f5ad54813628eb36e56b661422dfc36bb065cd51642e4af204f1ae6e`, byte-identical to the archived handoff.
- Optional nonplayable 32,768-byte GB/GBC diagnostic `GB-Chess-Search-EXPERIMENTAL-v0.7.0-diagnostic.gb`, SHA-256 `3d771ecc6f04934eeacb01b6e7080d3c09ee5092e174b7e7fa37e6675ebec126`, MD5 `20047f7e8aaa8fa6465bf0df79b11fd1`. The ROM is outside the source ZIP and must be paired with source and handoff.
- **Post-checkpoint baseline correction:** `GB-Chess-Search-v0.7.0-MAIN-BASELINE-ADDENDUM.md`, SHA-256 `a7977f31dc1af6bb70b34472792f549177f9d21d7b80dabdbaaeabb3722f8b40`. This updates the current main baseline to v1.3.0. The original v0.7.0 archive and matching handoff remain immutable, including their historical reference to v1.2.1.
- All four files are available in the GB Development ChatGPT conversation. Clean extraction verified all 65 file hashes and excluded ROMs, object files, emulator states, toolchains and nested archives. Deterministic repack reproduced the original ZIP SHA. Uncached GBDK rebuilt the exact ROM SHA/MD5, dual-mode flag $80, checked checksums, original DMG fallback, CGB double speed, and static WRAM end $C3A3.

## Old-fails/new-passes

At FEN `k7/8/1Q1K4/8/8/8/8/8 w - - 0 1`, frozen v0.6.0 depth-one search chose `b6c7` (+900), incorrectly overlooking that it stalemates Black. v0.7.0 scores a stalemate at the final searched ply as zero and chooses `b6d8` (+900), which retains Black's legal replies. The frozen v0.6.0 C source, regression and older evidence are preserved. Mate-in-one results `d4f2` and `g6g7`, each +29999, remain correct. Cancellation during an active nonchecked reply scan restores the search board and keeps a legal fallback.

A conservative positive-only pawn-reply witness skips the full cursor only when an unobstructed forward pawn push is known not to uncover a king attack. On 1,055 independently generated legal positions it yielded 1,014 positive certificates and no false positives against the separate Python oracle. Failure to certify is inconclusive and falls back to full reply scanning.

All **15 offline groups** passed after clean extraction, including historical canonical perft, 1,988 legal-position comparisons, 22 independent minimax positions, mate and cancellation regressions, pawn-witness tests and eight rejected evidence mutations. Targeted host UBSan passed.

The exact clean-built diagnostic passed **12 PyBoy 2.8.1 cold boots** (CGB/DMG x no input/A/B x two). The regenerated JSON is byte-identical to frozen evidence. Six positions cover the depth-two mate, opening cancellation, checkmated terminal, two depth-one mates and the new stalemate trap. Two-frame A/B pulses were retained. Search batches use at most six CGB or three DMG work units.

Sampled CGB/DMG elapsed frames: six-piece depth-two mate 339/698, stalemate trap 325/641. Maximum observed input-poll gaps 2/3 frames, single-step frame crossings 1/2 frames. An eight-unit CGB variant had a three-frame poll gap and was rejected; a four-unit variant was slower. These are emulator-specific position samples, not worst-case or physical-hardware bounds.

## Unfinished work

This is not a full FIDE reference or selected AI. Repetition history, prospective draw claims, 50-/75-move rules, dead positions, positional evaluation, rendering/audio load, SRAM/link and physical hardware need further testing or implementation. The separately promoted main v1.3.0 rules candidate must be recovered and compared before integration. The approved board-first storyboard and five planned modes remain unchanged. Trello stays Exploring.

Trello: https://trello.com/c/FYELcNLg

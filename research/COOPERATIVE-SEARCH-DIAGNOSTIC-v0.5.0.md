# Cooperative alpha-beta search diagnostic v0.5.0 (experimental)

Standalone, source-recoverable research only. Not a playable game or selected chess engine. Promoted project baseline remains v1.2.1 (ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`). The exact v1.2.1 source could not be materialized, so there is no integration into that tree. `master` and pinned upstream MIT engine are unchanged.

## Artifact identities

- Source ZIP: `GB-Chess-Search-EXPERIMENTAL-v0.5.0-source-and-evidence.zip`, 74,023 bytes, 44 entries, 43 matching file hashes. SHA-256 `96644a7eb0499b105c1b9ae11ba944fb9781547fab180d33291edd998662c1ce`.
- Standalone `GB-Chess-Search-EXPERIMENTAL-v0.5.0-STATUS-HANDOFF.md`: 8,766 bytes, SHA-256 `576ef0326ebe3832d7f2df865fda76dae8d9890576de0bc058b3e28f1af881cc`, byte-identical to archived handoff.
- Optional diagnostic `GB-Chess-Search-EXPERIMENTAL-v0.5.0-diagnostic.gb`: 32,768 bytes, SHA-256 `11c20ebc4e8c10d93cf0873a70a83b16ee1fc4fbd9f6d5d5b777bb0920b1f5e1`, MD5 `65364de62b182e46bf032f25fe4b4d17`. It is excluded from source ZIP and must only be distributed with matching source and handoff. Files are downloadable from the GB Development conversation.
- The clean extraction passed every manifest hash, forbidden-artifact audit and handoff comparison; deterministic ZIP repack reproduced identical bytes. The v0.4.0 source/handoff remain preserved, with three frozen historical search files retained here for a self-contained before/after regression.

## Verified development

- Explicit-state alpha-beta bounds now prune child search cursors while retaining nonrecursive search, legal fallback, completion-only depth publication, cooperative cancellation and exact position restoration. The unchanged rules core is independent of the pruning patch.
- Recompiled v0.4.0 unpruned reference versus new candidate: depth-two mate 3,082 -> 736 steps (76.1% fewer), depth-three mate 13,873 -> 5,565 (59.9%), opening depth two 1,870 -> 1,224 (34.5%), sparse fork depth two 968 -> 301 (68.9%). Mate answer/score remain `d4f2`/30,000. A source mutant that disables pruning fails the step ceiling.
- All 12 self-contained offline groups passed after clean extraction, including 20 fixed perft counts, 1,988 C/Python legal-move comparisons, 1,300 Python reversible plies, minimax reference scores on 22 more FENs, source mutations, and five deliberately corrupted new evidence records. Sampled host searches passed UBSan.
- The fixture batches up to eight logical search operations per input poll and stops a batch when sys_time advances. A clean GBDK build reproduced the exact ROM SHA and MD5, dual-mode CGB header `$0143=$80`, valid checksums, KEY1 double speed/DMG fallback and static WRAM end `$C2FC`. Twelve clean-source PyBoy cold boots (CGB/DMG x none/A/B two-frame pulses x two) matched the complete frozen evidence JSON byte-for-byte.
- Six-piece depth-two mate search: 201 CGB / 365 DMG simulated frames, versus unbatched/unpruned v0.4.0 3,082 / 3,125. Observed max input-poll gap increased from 1/2 to 2/3 frames (CGB/DMG), while largest search-step crossing was 1/2 frames. Legal B cancellation and root/internal board restoration verified. These figures are samples, not universal performance bounds.

## Limits

Still material-only depth-three search with no complete external FIDE adjudication, strong AI evaluation, dynamic-stack guarantee, physical console evidence, real UI/audio budget, SRAM/save/link or playable modes. Eight operations per poll is a logical cap; atomic move generation and sliding attack checks may cross frames. The approved board-first lesson storyboard and five future modes remain unimplemented. Recover the exact v1.2.1 archive before integration, then independently check rules and test bounded search during actual rendering/audio.

Project tracking: https://trello.com/c/FYELcNLg (Exploring).
# Cooperative chess search diagnostic v0.4.0 (experimental)

Status: standalone source-recoverable research add-on to the v0.3.0 rules prototype. Not a promoted gbchessdx v1.2.2 checkpoint, game, or selected chess engine. The latest promoted main research source remains v1.2.1 (ZIP SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`); its raw bytes were not available for integration. `master` and the pinned MIT `engine.c` are unchanged.

## Source recovery

- Source-and-evidence ZIP: `GB-Chess-Search-EXPERIMENTAL-v0.4.0-source-and-evidence.zip`, **59,915 bytes**, 34 entries, 33 file SHA-256 manifest entries. SHA-256: `a67b7ef0ecbe34b332c89667632778ea715ac8c1266cfd785959484c7f9bc0ad`.
- Matching standalone `GB-Chess-Search-EXPERIMENTAL-v0.4.0-STATUS-HANDOFF.md`, SHA-256 `cbc106a7849264763be04a40b308a5f2b60bc1a8fad329e9684f8479e0e41992`, byte-identical to the archived handoff.
- Optional diagnostic ROM: `GB-Chess-Search-EXPERIMENTAL-v0.4.0-diagnostic.gb`, 32,768 bytes, SHA-256 `c8e0742d0a64131d0b2cb7e8350454a6743c88ac3445019f2fbf28eafa46be49`, MD5 `4e907f70d8ea58196b79b7b0af083236`. Never distribute it without its matching source/handoff. It is excluded from the source ZIP.
- All three are downloadable in the GB Development ChatGPT conversation. Clean extraction passed every file hash, prohibited-artifact inventory and handoff comparison. Deterministic repack reproduced identical ZIP bytes. Original v0.3.0 rules and evidence are unchanged; the prior handoff is retained in `historical/`.

## Experiment and verification

- New `portable/search.c` and `.h`: explicit-state nonrecursive iterative deepening through 1-3 plies, using the previously tested cooperative legal-move cursor. Cancellation retains the first legal root fallback or last fully completed iteration, unwinds child positions and preserves the input board. Material-only leaves; interior mate/stalemate handling. This is a scheduling experiment, not a competitive chess AI.
- All **eight offline verification groups** passed from fresh extraction: existing perft, 1,988 C/Python exact legal-move comparisons, 1,300 Python game plies, cursor and evidence mutation checks, independent Python minimax/cancellation checks, three new rejected C search mutations, and three rejected new evidence mutations. Host C99 strict warnings and UBSan passed.
- Clean-source GBDK recompilation reproduced the exact dual-mode diagnostic ROM SHA and MD5 with valid checksums, header CGB flag $80, original DMG fallback, CGB KEY1 double-speed and linker static WRAM end $C2EF.
- **12 source-matched PyBoy 2.8.1 cold boots passed**: CGB/DMG x none/A/B x two matching boots. Entire regenerated JSON was byte-identical to frozen evidence. Completed depth-two six-piece mate chooses `d4f2` after 3,082 logical steps. Starting-board depth-three search is safely cancelled by a 180-frame watchdog or B pulse to legal `b1c3`; a two-frame A pulse is detected without cancelling. Checkmated position returns no move after 68 steps. Both working and original boards restore exactly.
- Observed largest search-step frame crossing: CGB 0, DMG 1. Input polling gap: CGB 1, DMG 2 frames. The 3,082-step mate takes 3,082 CGB or 3,125 DMG frames because one step is intentionally scheduled per VBlank. These are instrumented position-specific measurements, not worst-case/hardware bounds or acceptable gameplay times.

## Limits

No full FIDE certification, mature engine differential, repetition/draw search state, quiescence evaluation, UI/audio/stack stress, save/link, actual full chess game, physical console testing, strength rating or final AI selection. Move generation for a single square and attack-ray scanning are still indivisible. Reconcile the exact v1.2.1 source before shared integration; then optimize bounded work and independently benchmark tactical quality. Preserve the approved board-first lesson storyboard and five planned modes.

Trello: https://trello.com/c/FYELcNLg

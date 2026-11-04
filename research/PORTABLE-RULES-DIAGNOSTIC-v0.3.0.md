# Portable chess rules diagnostic v0.3.0 (experimental)

Status: separate, reproducible research add-on. **Not** a promoted gbchessdx v1.2.2 source, selected engine or playable game. The promoted v1.2.1 archive remains canonical (SHA-256 `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`). Its raw bytes were not materializable in this session. No changes to `master`, upstream MIT `engine.c`, final UI or gameplay modes.

## Exact recovery

- Source: `GB-Chess-Rules-EXPERIMENTAL-v0.3.0-source-and-evidence.zip`; 40,446 bytes, 23 entries, 22 independently validated per-file SHA-256 records. ZIP SHA-256: `1cb121bffdfd110b1d2a95a662a9b1a4feb8120e539e871b9d684558355a7bc3`.
- Standalone `GB-Chess-Rules-EXPERIMENTAL-v0.3.0-STATUS-HANDOFF.md`: SHA-256 `9242ded5408d3e6b01a21794a5539cd38f3e858a4055cb0a8a340b64cf9f7ae1`, byte-identical to archived handoff.
- Optional non-playable diagnostic: `GB-Chess-Rules-EXPERIMENTAL-v0.3.0-diagnostic.gb`; 32 KiB, SHA-256 `6030e4f27ed4515785d050c0b4a5d4b6560b87f4bad4a1bd4253d5eaf219cb1c`, MD5 `f26e89489d73faaa93764613c1a499cd`. Distribute only with matching source and handoff. Neither it nor any build artifacts are in the source ZIP.
- All three are available as downloadable attachments in the GB Development ChatGPT session. The complete archive was extracted cleanly, every manifest record verified, and a deterministic repack matched the ZIP byte-for-byte.

## Verified experiment

- Portable C99 legal-move engine, isolated from gameplay: king safety/pins, castling, en passant, promotions, reversible state, FEN counters. Full-board, per-piece, and cooperative pseudo-move/candidate-step interfaces. This is **not** a stronger or selected computer-chess search engine.
- 20 fixed canonical perft observations pass in C and the independent Python reference; 1,988 deterministic C/Python exact legal-move differentials, sampled cursor/piece equivalence, undo checks, 1,300 Python game plies, strict host C99 warnings and UBSan. A mutated cursor that drops legal knight moves fails.
- The same exact compiled ROM passes 12 matching PyBoy 2.8.1 fresh boots: CGB double-speed and original DMG, each with no-input, 2-frame A pulse, or 2-frame B pulse, twice. Six positions return counts [20,26,7,9,0,42] and unchanged board state across full/piece/cursor methods. Three forged evidence mutations are rejected.
- Observed maxima across these positions: whole-board 8/17, one piece 2/5, cursor step 1/2, input poll gap 2/3 frames (CGB/DMG). These are instrumented, position-specific frame counts, **not hardware or worst-case guarantees**. Static WRAM ends at $C597; dynamic stack remains unbounded.
- Two preserved old-fails/new-passes diagnostic fixes: a rook move falsely labeled as castling and a stale phase/case publication race that scheduled a DMG A pulse during blocking setup. No chess-rules bug was inferred from either failure.
- Fresh clean-source GBDK rebuild reproduced the exact ROM SHA, and 12 fresh boots reproduced the frozen JSON **byte-for-byte**.

Next: recover exact v1.2.1 raw source before integration; cross-check full FIDE rules against a separate mature rules engine; bound cursor work under realistic UI/audio, then evaluate genuinely resumable search and independently measured move strength. Hardware/Chromatic, save/link, lesson/story, and final engine remain unimplemented. Trello: https://trello.com/c/FYELcNLg

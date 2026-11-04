# Engine research v0.5.0: dual-mode CGB benchmarks

Research only. No new player-facing ROM has been promoted and no engine has been selected.

Trello: https://trello.com/c/FYELcNLg
Development fork: https://github.com/thisjustin816/gbchessdx
Branch: `research/engine-benchmark-v0.4` (default `master` unchanged)

## Recoverable source checkpoint

- Archive: `GB-Chess-Research-v0.5.0-source-and-evidence.zip`
- SHA-256: `3fce5003b28332251db3908222be43e1a58bd07225eb7010b8dc4287854c72b0`
- 666,676 bytes, 355 entries, 354 file hashes in `MANIFEST.json`.
- Clean extraction passed all six offline verification groups, including the 24 dual-mode result files.
- A fresh GBDK build from the extracted ZIP reproduced both exact fork-depth-2 fixture hashes and the saved results.
- The full source archive is downloadable in the GB Development ChatGPT project. This note is a recovery pointer, not a substitute for the ZIP.

## Verified search-only performance

Dual-mode cartridge header: `$0143=$80`, compiled with GBDK `-Wm-yc`. The test-only fixture uses `cpu_fast()` on GBC, and records `KEY1` before and after switching. PyBoy 2.8.1 was forced to `cgb=True` or `cgb=False`. Normal- and double-speed results and DMG fallbacks were repeated from cold boot.

| Puzzle | Move | GBC normal | GBC double | DMG fallback |
| --- | --- | ---: | ---: | ---: |
| Fork depth 2 | `g5f3` | 86 frames | 42 frames | 86 frames |
| Fork depth 3 | `g5f3` | 174 frames | 86 frames | 174 frames |
| Mate depth 2 | `d4f2` | 495 frames | 245 frames | 495 frames |
| Mate depth 3 | `d4f2` | 1,993 frames | 988 frames | 1,993 frames |

The timings measure emulated engine search only. They do not establish performance on physical hardware, full UI latency, or a ranking against Chessmaster. Double CPU speed does not double the display frame rate.

Reproduction sources: `modern-engine-research/cgb_dual_fixture.c`, `run_cgb_dual_fixture.py`, `build_and_check_cgb_dual.py`. The GBDK link uses `-Wm-yc -Wl-j -Wi-e`. `verify_cgb_dual_results.py` validates the saved evidence against v0.4's DMG baseline.

## Next

1. Measure actual `computer_move()` difficulty settings in dual-mode GBC double speed.
2. Expand tactical, rules/perft, defensive and endgame testing with an independent off-device chess oracle.
3. Measure memory/ROM headroom, cartridge save design, and GB/GBC/Chromatic behavior.
4. Retain the approved board-first storyboard layout. Final font, piece sprites and notation highlighting remain undecided.

No proprietary Chessmaster ROM or code is included in the research archive. Upstream `ncw/gbchess` engine files are pinned and unchanged.

## Active development guidance

Project archive loaded: `agents-main 4.zip`; SHA-256 `e97a4c1c5f4bff602dcf50bf7effe6f991cbf41b03970a29445224563163f2f3`. Read `gb-development`, `gb-rom-hacking`, `gb-source-builds`, and the PyBoy/hardware references. Use any newer project-provided archive for subsequent work.
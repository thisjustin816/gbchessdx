# Engine research v0.8.0: frame-bounded hint tests

Research checkpoint only. No selected engine, game ROM, gameplay UI, or changes to `master`. Project log: https://trello.com/c/FYELcNLg

## Source and recovery

- Source/evidence ZIP: `GB-Chess-Research-v0.8.0-source-and-evidence.zip`
- ZIP SHA-256: `5ae5b8d0b277deeb83dad7e261e638b0278fd8a995c58745af8306bd6690df00`
- 772,278 bytes, 441 ZIP entries, 440 manifest file hashes. All hashes and 12 offline verifier groups passed after clean extraction; no ROMs, saves, objects, emulator states or nested ZIPs.
- Standalone `GB-Chess-STATUS-HANDOFF-v0.8.0.md` SHA-256: `556e521f93cc90e9b1ddb64ade853ebf13c6a7f119f409413a69a020cb20a5a3`
- Both files are provided in the GB Development ChatGPT project conversation. The prior v0.7.0 ZIP is retained, SHA-256 `23a7aed019f92fda195c31b5ff5ba9749b88d5e9b9772bc9c3aade179c105cc0`.
- Upstream MIT `ncw/gbchess` `engine.c` Git blob `495890f379d449be5c051f8525034881574716ff` remains unchanged. Commercial Chessmaster ROMs are excluded.

## Timed analysis observations

A new chess-specific diagnostic reads GBDK `sys_time` (VBlank frame clock) inside the existing `engine_callback` while running real `computer_move(3)`. It prepares an independently checked legal fallback before search. Dual-mode 32 KiB test ROMs use CGB double-speed and DMG fallback; ROM headers, checksums, KEY1, WRAM, returned legality and position hashes are validated. Two matching PyBoy 2.8.1 cold boots per result.

| Position / console | Requested frames | Actual return frames | Move |
| --- | ---: | ---: | --- |
| Knight fork / CGB double | 16 | 23 | `g5e4`, misses fork |
| Knight fork / CGB double | 60 | 65 | `g5f3`, finds fork |
| Unique mate / CGB double | 16 | 18 | `d3d2`, misses mate |
| Unique mate / CGB double | 180 | 203 | `d3d2`, misses mate |
| Unique mate / CGB double | 280 | 290 | `d4f2`, mate |
| Unique mate / DMG normal | 280 | 283 | `d3d2`, misses mate |
| Pawn promotion / CGB double | 60 / 280 | 66 / 293 | `e2e3`, doesn't promote |
| Checked king / CGB double | 1 | 14 | legal `e1d2` |
| Checkmated king | 1 | 0 | no legal move; search skipped |

This is cooperative cancellation, **not a hard time limit**: engine callback polling allowed up to 23 frames of overshoot in this sample. The 280-frame mate budget finds mate on CGB but not DMG. Frame timing is simulated search/hint fixture timing, not live UI or physical hardware.

The checked-position's first generated pseudo-legal move would leave the king in check. A deliberate fault-injection test returns invalid `a1b1` with the guard disabled; with the guard enabled it replaces the injected candidate with legal `e1d1`. This is a wrapper safety regression, not an upstream engine defect. A restricted independent Python oracle verifies geometry and king safety for the sparse test FENs; it is not full FIDE validation.

Independent host fixed-depth tests on promotion FEN `7k/4P3/8/8/8/8/4K3/8 w - - 0 1`: depth 1 and depths 4–6 select `e7e8q`; depths 2–3 select `e2e3`. That is a teaching-quality concern, not an established engine bug or Elo comparison.

## Next

Prototype a genuinely cooperative game-loop analysis path with a visible thinking state, separate deadlines per console, and authored lesson solutions. Broaden legality and tactical/endgame references, compare modern engines and Chessmaster on equivalent positions, and test physical GB/GBC/Chromatic. Keep the approved board-first storyboard; fonts, pieces and highlighting remain deferred.

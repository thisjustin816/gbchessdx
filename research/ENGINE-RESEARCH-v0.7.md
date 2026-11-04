# Engine research v0.7.0 — bounded search and curriculum audit

**Research-only source/evidence checkpoint.** No new gameplay ROM, engine selection, player UI, or changes to `master`. Trello: https://trello.com/c/FYELcNLg

## Recoverable artifact

- `GB-Chess-Research-v0.7.0-source-and-evidence.zip`: **735,366 bytes**, **408 ZIP entries**, SHA-256 `23a7aed019f92fda195c31b5ff5ba9749b88d5e9b9772bc9c3aade179c105cc0`.
- 407 individual file hashes in `MANIFEST.json`, independently checked after clean extraction. No ROMs, save files, compile outputs, emulator states, toolchains, or nested ZIPs.
- Standalone `GB-Chess-STATUS-HANDOFF-v0.7.0.md` SHA-256 `f44aed5c2ebda076eb6ac2d76b97804d286f8d3b3752638b6030f2cd6a26834a`; byte-identical to archived `STATUS-HANDOFF.md`. Both downloads are available in the ChatGPT GB Development project conversation.
- Preceding *promoted* v0.6.0 archive remains separately preserved: SHA-256 `e5f65fcfca867fb7039b23e30362de2a1dd2f98ba6e05049073cd78a27f9ffec`. Its Library listing and standalone handoff were confirmed, but original archive bytes could not be materialized here. This v0.7 tree was rebuilt from the attached emergency source and independently validated, **not** byte-diffed against raw v0.6.
- Pinned upstream MIT `ncw/gbchess` `engine.c` Git blob `495890f379d449be5c051f8525034881574716ff` is unchanged. Reference commercial Chessmaster is used only for private behavioral evidence, never redistributed.

## Bounded real `computer_move(3)` results

Opt-in `engine_callback` instrumentation counts the engine's periodic ~16-node callback checks and requests cancellation at budget N; original fixture remains binary-identical at budget zero. The unrestricted six-piece Hard mate control is **censored after at least 3,232 search frames**, not crashed.

Six-piece unique mate `4k3/8/8/2b5/3q2b1/3p4/8/4K3 b - - 0 1`:

| Callback checks | GBC double-speed search frames | Chosen move |
| ---: | ---: | --- |
| 1 | 6 | `d3d2` (not mate) |
| 8 | 101 | `d3d2` (not mate) |
| 16 | 264 | `d4f2` (restricted oracle: `Qf2#`) |
| 32 | 321 | `d4f2` |
| 64 | 430 | `d4f2` |
| 16, original DMG mode | 532 | `d4f2` |

Four-piece fork `4k3/8/8/6n1/8/8/3Q4/4K3 b - - 0 1`: Hard at 16 callback checks chose `g5f3` (`Nf3+`) in **85 search frames**, vs 651 frames unrestricted. Two identical independent cold boots per mode/case (14 bounded JSON results), verified dual-mode `$0143=$80`, KEY1/DMG detection, exact ROM SHA-256, unchanged input position, legal output and expected moves. This is **PyBoy 2.8.1 simulated search-window timing**, not a real-time deadline, cartridge-hardware test, or strength/Elo claim. A one-position mate threshold does not generalize to arbitrary FENs.

The fixed callback latch resolves a diagnostic *counter assertion*, not an upstream chess-engine bug; historical failure preserved. All nine research/offline verifier groups passed after clean extraction, as did a fresh warning-clean GBDK rebuild of the 16-check mate: CGB 264 frames x2, DMG 532 frames x2, same ROM SHA-256 `71cd1599612acdc1fc22c8a621179f9dbc2e2e355f4315f0096d71f017909d7e`.

## Deep Research review

`analysis/deep-research-report-original.md`, independent `analysis/review_research_lessons.py`, JSON audit and `RESEARCH-REVIEW-v0.7.0.md` are included. Structured curriculum, progressive hints, repetition and explanation-based feedback are useful directions, not approved feature scope. Four of five supplied sample FEN/move explanations have board inconsistencies; none was imported into gameplay. The pawn-promotion example passes *restricted basic static checks only*. Prior approved board-first six-screen lesson layout remains the design direction; final font, art and highlighting are deferred.

## Next frontier

Test a **time-based**, frame/wall-clock bounded hint controller with safe pre-depth-1 fallback across more tactical, defensive and endgame positions; compare candidate engines with independent legal-move and strength references; then evaluate on physical DMG/GBC/Chromatic. No engine is selected. Keep `master` unchanged and project-specific fixture research here; generalized instrumentation belongs to Bacon Bit Bench.

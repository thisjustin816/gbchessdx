# GB Chess Academy — research checkpoint v1.2.1

**Research-only corrective continuation, not a playable chess ROM or selected game engine.** Published v1.2.0 is immutable: its ZIP was 895,901 bytes, 549 entries, SHA-256 `54a79fcf554c845b27ebda637a67df9432b0b77ac6f256d2bcf9a9e685967db0`. The new v1.2.1 checkpoint is additive and does not overwrite that earlier identity. No changes to `master` or pinned MIT chess engine source.

## Reproducible source and evidence
- Source/evidence ZIP: `GB-Chess-Research-v1.2.1-source-and-evidence.zip`, **917,428 bytes**, **575 entries**, **574 verified per-file SHA-256 hashes**.
- ZIP SHA-256: `ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`.
- Matching standalone `GB-Chess-STATUS-HANDOFF-v1.2.1.md`, **42,696 bytes**, SHA-256 `7412c7d0327bb8f4e98813fd205ea3395ca923d6191d5ca19db7b0e6738fac5d`. It is byte-identical to archived `STATUS-HANDOFF.md`.
- Clean extraction passed **all 574 manifest file hashes** and **18 offline verification groups**. Repackaging from the clean extraction produced identical ZIP bytes. No generated ROM, proprietary Chessmaster ROM, saves, emulator states, object files, toolchain, or nested ZIPs in the archive. Source and handoff downloads are in the GB Development ChatGPT project conversation.
- Clean, uncached warning-free GBDK rebuild reproduced the exact dual-mode 32 KiB diagnostic ROM SHA-256 `9eca6fd719ebeeb83544bf1405d87bc7352a186d5e6b9f71482292e6e8aa6191` (MD5 `89f000ec13a11e043c9ad088b05622b9`). Both GBC and DMG modes completed two identical cold boots from those same ROM bytes; each selected legal `a1d1` at a 180-frame cutoff.
- Prior v1.1 source ZIP SHA-256 `08967ad6ac63e90d30bf16a0b31f17e3362963d7c743f27c9c733a60b35b7c25` remains preserved. The raw published v1.2.0 ZIP was not byte-diffed during this recovery; the source was rebuilt from the available v1.1 plus staged v1.2 instrumentation and verified independently.

## Expanded stack and scanline experiment
13 configurations × two repeat-matched PyBoy 2.8.1 cold boots = **26 new source-matched JSON recordings**. The 20 earlier preliminary JSONs are explicitly archived under `stack-profile/legacy-evidence/` and not claimed rebuild-identical to this final source. The frozen verifier independently checks chess replay where supported, moves, position hashes, exact ROM hashes, console flags, marker bounds and timing, and rejects five deliberate mutations.

| Case | GBC double-speed | Original DMG |
| --- | --- | --- |
| Full 32-piece starting position | `b1c3`, 461 frames, max step 108 scanlines, stack marker 63 B | `b1c3`, 481 frames, 220 scanlines, 67 B |
| Full 27-piece developed position | `c4d3`, 1,954 frames, 118 scanlines, 63 B | `c4d3`, 2,005 frames, **240 scanlines**, 69 B |
| Developed 180-frame cutoff | `a1d1`, 180 steps, 63 B | `a1d1`, 175 steps, 67 B |
| Sparse mate | `d4f2`, 329 frames, 68 B | `d4f2`, 330 frames, 68 B |

Dynamic WRAM marker starts at `$C600`, with linked static data ending at `$C57D`, an active-SP-minus-16 guard, and a post-test scan **before** publishing the completion status. The deliberately pressured terminal control increased the observed marker from **60 B to 115 B**. An otherwise identical 90-frame search with guard 16 B detected 63 B, while guard 48 B detected **no marker changes**. Thus a zero reading means undetected, not zero stack use. The earlier premature-status false-negative was preserved as historical telemetry regression, not a chess-engine bug.

The longest observed DMG work unit was **240 scanlines** (more than one 154-scanline frame). Maximum recorded foreground input-sampling gap in these tests was 1 GBC or 2 DMG frames. These are **simulated, position-specific observations**, NOT universal worst-case/cycle-atomic/hardware-certified latency or stack bounds. The approximately 33-second completed developed search remains impractical for routine coaching hints. Upstream engine.c Git blob `495890f379d449be5c051f8525034881574716ff` unchanged.

## Project direction
Maintain the five requested modes as future design: Campaign with real chess bosses, Coaching with live hints/evaluation/undo, Puzzles, selectable VS Computer, and 2 Players (Pass & Play / Link Cable). The approved board-first 160x144 lesson layout, dual-mode DMG/GBC compatibility and GBC double speed remain requirements; no selected engine, save mapper, final story or player UI build. Next: FIDE-complete host rules, a stronger search that yields inside expensive operations, full-UI memory/stack budget, physical DMG/GBC/Chromatic testing and eventual SRAM/link validation.

Trello planning log: https://trello.com/c/FYELcNLg
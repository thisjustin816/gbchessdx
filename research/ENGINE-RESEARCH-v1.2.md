# GB Chess Academy — research checkpoint v1.2.0

Source/evidence research only. No selected engine, playable game ROM, game UI, save/link release, or changes to master. Trello: https://trello.com/c/FYELcNLg

## Source recovery
- Archive: GB-Chess-Research-v1.2.0-source-and-evidence.zip, 895,901 bytes, 549 entries, 548 manifest file hashes.
- ZIP SHA-256: 54a79fcf554c845b27ebda637a67df9432b0b77ac6f256d2bcf9a9e685967db0
- Standalone GB-Chess-STATUS-HANDOFF-v1.2.0.md SHA-256: 667b978364d4cae4e6b2e80e6a498b78c08b35c06307a63d1ba3aecec67eea2b; byte-identical to archived handoff.
- Clean extraction verified all 548 hashes and 18 offline test groups (including historical lesson report audit). Deterministic repack from that recovered source reproduced the exact same archive SHA. All generated ROMs, saves, objects, emulator states, toolchains, nested ZIPs and commercial Chessmaster ROMs are excluded.
- Fresh uncached warning-clean GBDK build from recovered source reproduced exact diagnostic ROM SHA-256 9eca6fd719ebeeb83544bf1405d87bc7352a186d5e6b9f71482292e6e8aa6191 and two identical DMG boots: legal a1d1 at 180 search frames, 67-byte stack watermark and 233-scanline max work unit.
- Previous promoted v1.1 ZIP retained separately SHA-256 08967ad6ac63e90d30bf16a0b31f17e3362963d7c743f27c9c733a60b35b7c25.
- Pinned MIT ncw/gbchess engine.c Git blob 495890f379d449be5c051f8525034881574716ff unchanged.

## New sampled stack and scanline profile
- A Game Boy LR35902-compatible SP reader primes unused WRAM from $C600 to (SP-16) with $A5; linker static WRAM ends $C57D (pressure-control variant $C57E). The scan runs before public completion status. A historical premature-status false-negative is preserved as old-fails/new-passes evidence.
- Positive control: a deliberate 96-byte volatile stack-local function increased the measured watermark from 60 to 115 bytes, confirming the detector responds to deeper calls.
- 10 conditions x 2 matching PyBoy 2.8.1 cold boots = 20 frozen JSONs with ROM hashes, position invariance, fixed moves, console KEY1, stack addresses, LY scanline maxima, and 3 negative mutations. Restricted independent chess replay validates the developed board and its two explicit castling moves.
- Original starting position, full shallow search: CGB double b1c3 at 461 frames / 108 max scanlines / 63-byte watermark; DMG b1c3 at 481 frames / 220 max scanlines / 67-byte watermark.
- Developed 27-piece board, full shallow search: CGB c4d3 at 1954 frames / 118 max scanlines / 63-byte watermark; DMG c4d3 at 2005 frames / 240 max scanlines / 69-byte watermark. The instrumented DMG result is 5 frames slower than the prior v1.1 uninstrumented 2000-frame result.
- Additional DMG developed controls: a two-frame B press, scheduled 20 frames after actual search start, safely cancels and returns legal a1b1 after 22 frames; a 180-frame watchdog returns legal a1d1. Both preserve position hash. Mate returns d4f2 at 329/330 CGB/DMG frames with 68-byte watermarks.
- This is *diagnostic whole-call-path* stack usage, not isolated engine stack consumption or a hardware-certified upper bound; $A5 collisions can undercount. The 240-scanline DMG maximum can cross a 154-line display frame, and these observations are not worst-case guarantees for arbitrary positions or complete game UI.

## Architecture direction
- Two-ply sliced search proves foreground responsiveness but can take ~33s for a dense move and misses known tactical goals. It is NOT selected as the strong VS/Coaching/Boss engine.
- Proposed next: host-tested full-FIDE rules and state/undo, resumable iterative deepening with yields *within* move generation/evaluation, separate authored puzzle answers from generic hints, independently calibrated AI difficulties, full UI memory/stack budget, and physical DMG/GBC/Chromatic testing.
- Preserve five user-requested modes as design requirements, NOT shipped features: Campaign (lessons and real chess bosses); Coaching (hints, move feedback, takeback); Puzzles; VS Computer (selectable level); 2 Players (Pass & Play or Link Cable). Approved board-first lesson layout remains unchanged; final story/art/save architecture still open.

Source details: modern-engine-research/sliced-search/stack-profile/README.md and analysis/ENGINE-ARCHITECTURE-v1.2.md inside the matching recoverable archive.
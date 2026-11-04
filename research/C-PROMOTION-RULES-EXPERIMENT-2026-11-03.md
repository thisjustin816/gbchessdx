# C promotion-rule candidate — host experiment (2026-11-03 Pacific)

**Experimental add-on only.** This is NOT promoted v1.2.2, NOT a playable ROM, NOT a chosen chess AI, and NOT a replacement for the separately promoted v1.2.1 source. `master` and the pinned MIT upstream `engine.c` Git blob `495890f379d449be5c051f8525034881574716ff` remain untouched.

## Recoverable source

- Bundle: `GB-Chess-C-Promotion-EXPERIMENTAL-2026-11-03.zip`
- ZIP SHA-256: `299ef8e7901f4f7b01eca253e227f38a4d3d451f09129a3ffd9e55e060bf5afc`
- `STATUS-HANDOFF.md` SHA-256: `577112c301d5911134b75e08033ebb6704c32d372f36a466f2fa122b8951d616`
- 31,137 bytes; 16 entries; all 15 manifest records and ZIP/extracted bytes verified; deterministic clean-extraction repackage gives the identical ZIP.
- Bundle includes unchanged pinned `chess.c`, `eval.c`, headers, `engine.c`, upstream MIT license, an optional patch, C99/UBSan host perft runner, verification scripts and frozen JSON evidence. No ROM, proprietary code, generated object or emulator state.
- Source was recovered from the user's exact v1.2.0 ZIP (`54a79fcf554c845b27ebda637a67df9432b0b77ac6f256d2bcf9a9e685967db0`); **newer promoted v1.2.1** (`ea6e532e52dbe235d2ebda7ae5af2e7a8c654d9c467cd13e03c3fc2c26f7829f`) exists separately, but its raw bytes could not be materialized in this session. Do not merge/archive this as v1.2.1.
- Full source/evidence archive and matching handoff are available in the ChatGPT GB Development project conversation.

## Verified findings

A six-FEN host perft harness checks externally published reference counts, move enumeration, full upstream working-state restoration after make/unmake, pinned source identity, strict GCC warnings, UBSan and negative mutations.

- Pinned gbchess matches **start 20/400/8902**, **Kiwipete 48/2039/97862**, **position 3 14/191/2812**, **position 6 46/2079/89890** through depth 3.
- Pinned source fails standard promotion positions: position 4 has depth-2/3 **228/8087** vs reference **264/9467**; position 5 has depth-1/2/3 **41/1373/54007** vs **44/1486/62379**. It hardcodes queen promotion; three underpromotions are absent from root move choices in position 5.
- The **temporary C patch** adds Q/R/B/N promotion options through `Move.special` values 3..6, updates make/unmake/evaluation, and matches **all 24 published perft counts across the six positions at depths 1–4**, including Kiwipete **4,085,603** and position 5 **2,103,487**.
- A source mutation that drops knight promotion is rejected; pinned source stays byte-identical. Both verifiers passed from a fresh extraction.

## Important limits

This is not a complete FIDE rules engine or proof of all edge cases: draw adjudication/repetition history, promotion selection UI, high-branching `MAX_MOVES=80` capacity, full AI compatibility, cartridge memory/timing, DMG/CGB/Chromatic, Link, Save and gameplay are untested. An earlier separate GBDK rebuild attempt timed out during compilation **before** ROM creation; do not describe it as a ROM failure or identical rebuild.

The separate Python rules host candidate already recorded at `research/FIDE-RULES-HOST-CANDIDATE-2026-11-03.md` is a distinct experiment. Next: recover v1.2.1 source bytes, compare both candidate rules paths on identical game histories, expand draw and promotion API coverage, verify move-list worst cases, then integrate and test on GB/GBC only after new source/ROM recovery gates.

References: https://www.chessprogramming.org/Perft_Results and https://grandchesstree.com/perft-test . Trello: https://trello.com/c/FYELcNLg

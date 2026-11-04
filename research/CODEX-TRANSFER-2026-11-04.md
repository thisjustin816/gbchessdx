# GB Chess Academy - Codex transfer checkpoint (2026-11-04)

**Transfer prepared; source-integrity verified. NOT a game build or promotion.** Use the downloadable attachments in the GB Development ChatGPT conversation. Do not treat this GitHub note as a replacement for the source ZIPs.

- All-in-one transfer: `GB-Chess-Codex-Transfer-2026-11-04.zip`, **1,424,360 bytes**, 41 members, SHA-256 `bbabfcdaa0a86699f3c15df92b3cd3b341572643b98c8f28023e211f876fdd85`. All 40 file entries are individually hashed in `TRANSFER-MANIFEST.json`; the deterministic repack was byte-identical.
- Standalone transfer handoff: `GB-Chess-Academy-CODEX-HANDOFF-2026-11-04.md`, SHA-256 `1cdd391a104b8a8be3ab77e870caa35b37ef321724c5983e4f71055ee07a8139`.
- Copyable first prompt: `GB-Chess-CODEX-FIRST-PROMPT-2026-11-04.txt`, SHA-256 `2fe8d5f6fbadf99473b65ed2cceeaa241c35eb69b3f451ff220ca8c006652e8e`.
- Verified, source-matched standalone cooperative search **v0.7.0** ZIP SHA-256 `f4ed65c3373a4c8ca70f2a2f3480f9013905364f16255abc3b2dc741ac0e0208`; matching nonplayable ROM SHA-256 `3d771ecc6f04934eeacb01b6e7080d3c09ee5092e174b7e7fa37e6675ebec126`. This remains the independently verified research fallback, NOT the main game.
- New **unverified** v0.8.0 source-only WIP ZIP, SHA-256 `16d122f4b87743eb5212ab484290860d15b5d66de09f33fa0448149487b28bf1`. It includes a 75-move draw search candidate and 77 hashed source/evidence files. Targeted `verify_seventyfive.py` **PASS**; frozen `verify_seventyfive_evidence.py` **FAIL** on the `depths` array. Failing log saved inside WIP; no v0.8 ROM delivered or approved.
- Historical main v1.2.0 exact archive included for context only, SHA-256 `54a79fcf554c845b27ebda637a67df9432b0b77ac6f256d2bcf9a9e685967db0`.
- The separately promoted **MAIN v1.3.0** source ZIP (expected SHA-256 `e604a07e57f9e63a51b59f3694c8bf8d19b3ae696087b35768cf64fee1f65cf0`) and matching handoff are NOT available here. **The user cannot supply missing files; do not ask. Do not reconstruct, falsely verify, merge, or claim main v1.3.0 integration.** Continue in an isolated research branch using verified v0.7.0 while keeping this limitation explicit. The v1.3.0 research note and original Trello promotion remain separate.
- `master` and pinned upstream MIT `engine.c` must remain unchanged. The existing branch `research/engine-benchmark-v0.4` contains notes, not a full archive mirror. The transfer includes project instructions and three bundled GB development skills. No commercial Chessmaster ROM is shared.

**First Codex task:** Verify transfer and v0.7 manifest/checkpoint; run its clean offline and dual-mode emulator regression; independently reproduce v0.8 host PASS versus evidence FAIL, identify the first divergence before changing expectations, then complete tests and source-recoverable checkpoint only if promotion gates pass. Keep the final AI unselected and game modes/UI unchanged. Trello stays Exploring.

Trello: https://trello.com/c/FYELcNLg

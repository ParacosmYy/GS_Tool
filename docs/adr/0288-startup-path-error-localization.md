# ADR-0288: Startup-path error localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D244 / UI-26

## Decision

Extend the existing presentation-only `localize_message()` compatibility
mapper with the two missing prefixes emitted when explicit desktop launch
paths cannot be admitted: `Cannot open path: ` and `Unable to open path: `.
The `zh-CN` projection translates only the prefix; the requested path and
underlying exception text remain intact as diagnostic details. `en-US`
returns the original message unchanged.

No new translation service, startup parser, document-store behavior, or Qt
translator is introduced. The existing status and message surfaces continue
to own locale projection.

## Public-source applicability

No external vendor or manufacturer requirement is applicable to this project-
local catalog mapping. The implementation does not change a Qt API or a
platform contract; it preserves the existing presentation catalog and
`localize_message()` boundary. This is an engineering/project rule, not a
ByteDance internal standard, certification claim, MISRA/ISO 26262/ASPICE claim,
or manufacturer requirement. Embedded C/C++, MCU, BSP/HAL, RTOS, and related
public-source workflows are not applicable.

## Evidence and limits

- `D244-COMPILEALL=PASS`; `D244-RUFF=PASS`; `D244-FORMAT=PASS`.
- `D244-STARTUP-PATH-LOCALIZATION=PASS`: both prefixes translate in `zh-CN`,
  remain unchanged in `en-US`, and retain suffix details.
- PS5.1 build passed with intermediate SHA
  `016B0F974E0EA3B975B60DD729F4DD33A4B339ACB8912772F6754B03D1B7BF4D`;
  PS7 final build passed with SHA
  `35A0D6EDDDE7E5BF0BB79E7FCA1EFD30B6580C29062CFB5D8821599213E5ECE7`,
  38,576,104 bytes.
- Final source revision:
  `tree-sha256:66d2d6529b40ab77549dd4c278651a83f588cdf29c4d399fc7c42eceb9d69ba1`.
- Recursive frozen archive contains `__main__`, `quillforge.app`,
  `quillforge.composition`, PyQt6, and `qwindows.dll`; project and handoff
  checks remain static/non-destructive.
- Parent review and simplification assessment: `PASS`.
- Pasteur the 7th architecture and Cicero the 7th independent Luna/max
  windows returned `NO_CONCLUSION` after bounded waits; no child PASS is
  claimed.
- Native EXE/Qt startup, desktop file association behavior, and clean-machine
  rendering remain unrun under the permanent no-launch policy.

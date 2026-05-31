# FEATURE-042B: Modbus RTU Frame Template

## Motivation

The FrameParser state machine and FrameVisualEditor are fully implemented, but users
must manually define every frame format (header, length, checksum, fields) before
they can parse any data. For the most common embedded protocol -- Modbus RTU --
this means each user independently re-creates the same frame definition. This is
friction that pushes users toward specialized Modbus tools instead of using EmbedDebug.

Adding a Modbus RTU template gives instant value to a large user segment (industrial
MCU developers, PLC integrators, sensor network engineers) and demonstrates the
power of the existing FrameParser infrastructure without requiring any changes to the
parser itself.

## User Story

As an embedded developer communicating with Modbus RTU slave devices over serial,
I want to select "Modbus RTU" from a template dropdown and immediately start reading
Holding Registers or Coils, so that I can verify device responses without manually
configuring frame headers, CRC16, and field offsets every time.

## Technical Approach

### Modules Affected
- `protocol/FrameDefinition.h/cpp` -- add JSON serialization for templates (already exists)
- `protocol/FrameVisualEditor.h/cpp` -- add "Load Template" dropdown button
- New file: `protocol/FrameTemplateLibrary.h/cpp` -- template registry and preset definitions
- New file: `resources/templates/modbus_rtu.json` -- Modbus RTU frame definition
- `utils/CRC.h` -- already has CRC16-Modbus (reused directly)

### Template Structure
A template is simply a pre-populated `FrameDefinition` serialized to JSON, bundled
with a metadata wrapper:

```json
{
  "name": "Modbus RTU Response",
  "category": "Industrial",
  "description": "Standard Modbus RTU response frame (slave address + FC + data + CRC16)",
  "frame": {
    "header": "",
    "footer": "",
    "lengthFieldOffset": 2,
    "lengthFieldSize": 1,
    "checksumType": "CRC16Modbus",
    "checksumOffset": -1,
    "checksumSize": 2,
    "fields": [
      {"name": "SlaveAddr", "offset": 0, "size": 1, "type": "UInt8"},
      {"name": "FunctionCode", "offset": 1, "size": 1, "type": "UInt8"},
      {"name": "ByteCount", "offset": 2, "size": 1, "type": "UInt8"},
      {"name": "Data", "offset": 3, "size": 0, "type": "Raw"}
    ]
  }
}
```

### Implementation Strategy
1. **FrameTemplateLibrary**: Singleton registry. On construction, loads all JSON files
   from `resources/templates/` and exposes `availableTemplates()` and
   `loadTemplate(name) -> FrameDefinition`.
2. **FrameVisualEditor**: Add a "Templates" QComboBox in the toolbar above the editor.
   When user selects a template, the editor populates all fields from the loaded
   FrameDefinition. User can then modify field names/offsets for their specific use case.
3. **Bundled templates**: Ship with Modbus RTU (request + response) and one simple
   "Custom TLV" template as a starting point. Future iterations add Modbus ASCII,
   CAN-Open, etc.
4. **User templates**: FrameTemplateLibrary also scans
   `SettingsManager::dataDir() + "/templates/"` for user-created JSON files, enabling
   custom template sharing across sessions.

### Complexity Estimate
- Low-to-Medium. ~300 lines new code. The heavy lifting (FrameParser, CRC16-Modbus,
  FrameDefinition JSON serialization) already exists and is fully tested.

## Architecture Impact

### New Classes
| Class | Layer | Responsibility |
|-------|-------|---------------|
| `FrameTemplateLibrary` | Data | Load/registry of frame definition templates from JSON files |

### Pattern Changes
- Introduces a simple registry pattern. No impact on existing patterns.
- FrameVisualEditor gains a template-selection UI path. No structural change.

### Existing Component Reuse
- `FrameDefinition::fromJson()` -- direct reuse for template deserialization
- `FrameVisualEditor` -- just adds a combo box, calls existing `setDefinition()`
- `CRC.h` -- CRC16Modbus already implemented
- `SettingsManager` -- user template directory path

## Development Cost

| Item | Estimate |
|------|----------|
| FrameTemplateLibrary | ~100 lines |
| FrameVisualEditor template UI | ~60 lines |
| JSON template files (Modbus RTU x2, TLV x1) | ~90 lines |
| QSS additions for template combo | ~15 lines |
| **Total** | **~265 lines** |
| **Iterations** | **1 iteration** |

## Priority Justification

P2 from the candidate pool ("Multi-language frame templates"), but scoped down to
Modbus RTU only for a single-iteration delivery. Modbus is the most widely used
industrial serial protocol and the most common reason users reach for a dedicated
protocol tool instead of a general serial terminal. This feature directly leverages
the existing FrameParser + FrameVisualEditor + CRC infrastructure with zero changes
to those modules, demonstrating that the architecture supports rapid feature addition.

The template infrastructure (FrameTemplateLibrary + user template directory) also
serves as the foundation for future protocol templates (Modbus ASCII, CAN-Open,
custom TLV), making each additional template a near-zero-cost addition.

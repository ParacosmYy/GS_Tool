# UI/UX Audit Report -- Iteration 47

**Reviewer**: UI/UX Product Experience Reviewer
**Date**: 2026-06-01
**Scope**: 3 theme files + 5 C++ source files
**Current Score**: 46/1000

---

## Category Scores

| # | Aspect | Score (1-10) | Verdict |
|---|--------|:---:|---------|
| 1 | Color consistency | **6** | Semantic palette defined, but hardcoded values still pervasive in QSS rules |
| 2 | Button states | **7** | Most buttons have hover/pressed/disabled; some gaps in edge-case selectors |
| 3 | Input states | **6** | Normal/focus/error/disabled covered for QLineEdit; SpinBox and ComboBox focus visual missing |
| 4 | Spacing | **5** | Mixed margin values (12 vs 8), padding inconsistencies across panels |
| 5 | Typography | **5** | Font sizes inconsistent (11/12/13/14px), no scale system, monospace declared without fallback chain |
| 6 | Animation | **6** | Search bar and OTA progress animated; button hover, panel slide, theme fade missing |
| 7 | Dark/Light theme parity | **4** | Structural parity good, but semantic color drift and missing selectors in light theme |
| 8 | Status indicators | **8** | Connection dot + breath animation + state-driven QSS is well done |
| 9 | Navigation | **5** | No panel transition animation in code; nav tree QSS exists but C++ side silent |
| 10 | Overall polish | **5** | Functional but feels "configured" not "designed" -- lacks the cohesion of Linear/Raycast |

**Total**: 57/100

---

## 1. Color Consistency (Score: 6/10)

### 1.1 Semantic palette is defined but not consumed by QSS

**What is right**: All three themes define a semantic palette in CSS comments (lines 17-41 in dark_terminal.qss, 15-39 in modern_dark.qss, 15-39 in light.qss). ThemeManager parses these for self-drawn widgets.

**What is wrong**: The QSS rules themselves still hardcode every color value. There is no DRY mechanism -- the same `#cdd6f4` appears 50+ times in dark_terminal.qss. If the designer wants to adjust `--text-primary`, they must find-and-replace across 50 locations with risk of missing one.

**Impact**: Maintenance nightmare. Theme authors must manually synchronize color values across hundreds of selectors.

**Fix**: This is a QSS limitation (no CSS variables), but the workaround is to adopt a preprocessor or generate QSS from a template. For now, at minimum, add a comment block at the top mapping semantic names to hex values used in the file:

```css
/*
 * Color Reference (dark_terminal.qss):
 * TextPrimary:   #cdd6f4  (used in ~50 selectors)
 * TextSecondary: #a6adc8  (used in ~20 selectors)
 * ...
 */
```

### 1.2 Inconsistent color values across themes for same semantic role

**File**: `modern_dark.qss` line 524 vs `dark_terminal.qss` line 530

`modern_dark.qss` uses `#565f89` for driverInfoLbl color. `dark_terminal.qss` uses `#6c7086`. Both represent "text-muted" but use different hex values. This is intentional per-theme variation, but the gap between them is significant -- `#565f89` is notably more readable than `#6c7086` at small font sizes. The dark_terminal theme's muted text is harder to read.

**Fix**: Increase contrast in dark_terminal.qss muted text. Change from `#6c7086` to something closer to `#7f849c`:

```css
/* dark_terminal.qss line 387 */
QLabel {
    color: #bac2de;    /* current: borderline, consider #c0c8de for better readability */
    font-size: 13px;
}
```

### 1.3 Send input color is hardcoded green in QSS

**File**: `dark_terminal.qss` line 135

```css
QLineEdit {
    color: #a6e3a1;   /* This is --Success, not --TextPrimary */
}
```

The global QLineEdit color is set to the success/green color `#a6e3a1`. This makes ALL LineEdits green-texted, including the search bar input, bookmark label input, and OTA file path. Only the send input should be green (to distinguish TX data). Other LineEdits should use `--text-primary`.

**Fix**: Scope the green color to the send input only:

```css
/* Remove color from global QLineEdit, use specific selector */
QLineEdit {
    background-color: #181825;
    color: #cdd6f4;                    /* text-primary, NOT green */
    border: 1px solid #45475a;
    ...
}

/* Send input gets the green TX color */
QLineEdit#sendInput {
    color: #a6e3a1;
}
```

Apply same fix in `modern_dark.qss` (line 131-132) and `light.qss` (line 131-132).

---

## 2. Button States (Score: 7/10)

### 2.1 Missing hover color change for many secondary buttons

**File**: `dark_terminal.qss` line 169

```css
QPushButton:hover {
    background-color: #45475a;
    /* Missing: no border or color change */
}
```

The generic QPushButton hover only changes background. There is no border-color shift or text color shift. Compare with the quickCmdBtn hover (line 275) which adds `border-color: #89b4fa; color: #89b4fa;`. The generic button feels lifeless on hover -- the user gets a barely-perceptible gray shift.

**Fix**:

```css
QPushButton:hover {
    background-color: #45475a;
    border-color: #585b70;     /* subtle border brightening */
}
```

### 2.2 connectBtn[state="error"] missing QSS

**File**: `dark_terminal.qss` line 506-528

The connectBtn has selectors for `state=""`, `state="connected"`, and `state="connecting"`, but there is no `state="error"` selector. When `SerialConfigPanel::setError()` is called (line 210-236 of SerialConfigPanel.cpp), it sets `setProperty("state", "error")` but the QSS has no matching rule. The button falls back to the default QPushButton style, losing the error visual.

**Fix**: Add to all three theme files:

```css
QPushButton#connectBtn[state="error"] {
    background-color: #f38ba8;
    color: #1e1e2e;
}
QPushButton#connectBtn[state="error"]:hover {
    background-color: #eba0ac;
}
```

### 2.3 searchBarCloseBtn hover and pressed are identical

**File**: `dark_terminal.qss` line 682-683

```css
QPushButton#searchBarCloseBtn:hover { background: #89b4fa; }
QPushButton#searchBarCloseBtn:pressed { background: #89b4fa; }
```

Hover and pressed states are the same color. The user gets no tactile feedback that their click registered.

**Fix**:

```css
QPushButton#searchBarCloseBtn:hover { background: #74a8f7; color: #1e1e2e; }
QPushButton#searchBarCloseBtn:pressed { background: #5c96e5; color: #1e1e2e; }
```

---

## 3. Input States (Score: 6/10)

### 3.1 QLineEdit focus state uses 1px border -- should be 2px for clarity

**File**: `dark_terminal.qss` line 143-145

```css
QLineEdit:focus {
    border: 1px solid #89b4fa;
}
```

The CLAUDE.md section 6.6 specifies: "Focus: 2px border-focus color border + slight outer glow". All three themes use 1px. The error state correctly uses 2px, making the focus state visually weaker than the error state.

**Fix**: All three theme files, change QLineEdit:focus:

```css
QLineEdit:focus {
    border: 2px solid #89b4fa;
    padding: 5px;    /* compensate for thicker border */
}
```

Apply to `QLineEdit#searchBarInput:focus` (dark_terminal.qss line 648) and `QLineEdit#bookmarkLabelInput:focus` (dark_terminal.qss line 1277) as well.

### 3.2 QSpinBox has no focus state in any theme

**File**: `dark_terminal.qss` line 333-354

SpinBox defines hover and disabled but the `:focus` selector (line 346-348) only changes border to `1px solid #89b4fa` -- same thickness as the default border. The focus indicator is nearly invisible.

**Fix**: Increase focus border to 2px:

```css
QSpinBox:focus {
    border: 2px solid #89b4fa;
    padding: 3px;   /* reduce padding to maintain size */
}
```

### 3.3 ComboBox has no focus state at all

**File**: `dark_terminal.qss` line 279-315

ComboBox defines `:hover` (border color change) and `:disabled`, but there is no `:focus` selector. When the user tabs to a ComboBox, there is zero visual indication of focus. This fails WCAG 2.4.7 (Focus Visible).

**Fix**:

```css
QComboBox:focus {
    border: 2px solid #89b4fa;
}
```

### 3.4 QComboBox QAbstractItemView has no hover state for items

**File**: `dark_terminal.qss` line 317-325

The dropdown popup has `selection-background-color` but no `::item:hover` state. Users cannot preview which item they will select before clicking.

**Fix**:

```css
QComboBox QAbstractItemView::item:hover {
    background-color: #45475a;
}
```

---

## 4. Spacing (Score: 5/10)

### 4.1 Inconsistent content margins across panels

| File | Panel | Margins |
|------|-------|---------|
| SerialConfigPanel.cpp:41 | mainLayout | `12, 12, 12, 12` |
| QuickCommandBar.cpp:16 | mainLayout | `8, 6, 8, 6` |
| OtaWidget.cpp:62 | mainLayout | `12, 12, 12, 12` |
| FrameVisualEditor.cpp:54 | mainLayout | `12, 8, 12, 8` |
| TerminalSearchBar.cpp:51 | layout | `8, 4, 8, 4` |

CLAUDE.md section 6.3 says panel inner padding should be 8-12px. These are within range, but the inconsistency (12 vs 8 vs mixed) creates subtle visual misalignment when switching between panels. The FrameVisualEditor uses `12, 8, 12, 8` (asymmetric), while others use uniform values.

**Fix**: Standardize all panels to `12, 12, 12, 12` for main panel content and `8, 4, 8, 4` for embedded bars (search, quick commands). FrameVisualEditor.cpp line 54:

```cpp
mainLayout->setContentsMargins(12, 12, 12, 12);
```

### 4.2 QGroupBox margin-top inconsistency between QSS and C++

**File**: `dark_terminal.qss` line 392-400

```css
QGroupBox {
    margin-top: 14px;
    padding-top: 18px;
}
```

But the FrameVisualEditor-specific groups (line 756-766) override:

```css
QGroupBox#frameHeaderGroup, ... {
    margin-top: 12px;
    padding-top: 16px;
}
```

Two different GroupBox styles in the same theme. The 2px difference is noticeable when FrameEditor groups sit next to SerialConfig groups.

**Fix**: Unify all QGroupBox to the same margin-top/padding-top:

```css
QGroupBox {
    margin-top: 14px;
    padding-top: 18px;
}
/* Remove the overrides from frame-specific selectors */
```

### 4.3 QuickCommandBar button sizing uses different strategies

**File**: QuickCommandBar.cpp line 29 vs line 37 vs line 112

- Edit button: `setMinimumHeight(32)` -- correct, flexible width
- Add button: `setMinimumHeight(32)` -- correct
- Dynamic command buttons: `setMinimumSize(80, 32)` + `setMaximumWidth(160)` -- correct

But the edit button has no minimum width, so at narrow window sizes it could collapse to just "Edit" text with no side padding. Also `m_editBtn` and `m_addBtn` have no `setMinimumWidth` set.

**Fix**: Add minimum width to edit and add buttons:

```cpp
m_editBtn->setMinimumWidth(60);
m_addBtn->setMinimumWidth(40);
```

---

## 5. Typography (Score: 5/10)

### 5.1 Font size chaos -- at least 6 different sizes in use

Sizes found across all three themes:
- 11px: driverInfoLbl, otaFileInfo, rxRateLabel, txRateLabel, chartStatusLabel
- 12px: QStatusBar, quickCmdEditBtn, quickCmdBtn, QComboBox drop-down, QPushButton (secondary), QGroupBox title, otaLogView, protocolTable, frameFieldTable, header sections
- 13px: QTreeView, QToolButton, QLineEdit, QPushButton (primary), QComboBox, QSpinBox, QCheckBox, QLabel, QGroupBox
- 14px: connectBtn, quickCmdAddBtn
- 16px: quickCmdAddBtn font-size

That is five sizes. CLAUDE.md section 6.4 says "UI font size 12-13px, label 12px, group title 13px". The 11px labels are below the minimum. The 14px connectBtn and 16px add button break the scale.

**Fix**: Establish a type scale and stick to it:

```
11px -> ELIMINATE (use 12px minimum for readability)
12px -> Secondary labels, status bar, table cells, toolbar buttons
13px -> Primary text, inputs, body content
14px -> DO NOT USE (remove from connectBtn, use 13px + font-weight: bold instead)
16px -> DO NOT USE for regular UI (keep only for the "+" add button as an icon surrogate)
```

### 5.2 Monospace font declaration lacks proper fallback

**File**: dark_terminal.qss line 139

```css
font-family: "Consolas", "Courier New", monospace;
```

This is acceptable but misses "JetBrains Mono" which CLAUDE.md section 6.4 specifies as a preferred terminal font. Also, the `monospace` generic family behaves differently across platforms.

**Fix**:

```css
font-family: "JetBrains Mono", "Consolas", "Courier New", monospace;
```

### 5.3 FrameVisualEditor preview label hardcodes font in C++

**File**: FrameVisualEditor.cpp line 173

```cpp
m_previewLabel->setFont(QFont("Consolas", 10));
```

This violates CLAUDE.md section 6.8 rule 1: "No hardcoded font values in C++ code." The font family and size should come from QSS.

**Fix**: Remove the C++ font call and add a QSS rule:

```css
QLabel#framePreviewLabel {
    font-family: "JetBrains Mono", "Consolas", "Courier New", monospace;
    font-size: 12px;
    ...
}
```

The QSS already defines this (dark_terminal.qss line 830-846), so the C++ line is redundant AND creates a conflict (QSS says 12px, C++ says 10px). The C++ wins over QSS for font properties, so the label renders at 10px -- smaller than everything else.

---

## 6. Animation (Score: 6/10)

### 6.1 Button hover/pressed transitions missing

QSS does not support CSS transitions natively. The button state changes are instant (no fade). CLAUDE.md section 6.5 lists "Button hover gradient" as a required animation with 200ms OutCubic. None of the three theme files define any transition properties (Qt does not support them in QSS).

This is a fundamental limitation. The workaround is to use QPropertyAnimation on custom properties in C++ code, or accept instant state changes. Given that Qt 6.8.3 does not support QSS transitions, this is partially excusable.

**Partial fix**: For the most important buttons (connect, send, otaStart), consider a custom QAbstractButton subclass with animated hover, or accept the limitation and document it.

### 6.2 Panel switching has no animation code

**File**: SerialConfigPanel.cpp -- the entire file

There is no panel transition animation code anywhere in the reviewed C++ files. CLAUDE.md section 6.5 lists "Panel slide in/out" as P0 priority. NavigationController presumably handles this, but none of the reviewed files participate.

The panel contents just appear and disappear. When switching from "Serial Config" to "OTA", there is no slide, no fade, nothing. This is a major UX gap.

### 6.3 Theme switching has no transition

None of the reviewed files implement a theme-switch fade animation. When the user selects a different theme in the toolbar, all colors change instantly. CLAUDE.md section 6.5 lists "Theme switch fade" as P0.

### 6.4 Search bar animation is well implemented

**File**: TerminalSearchBar.cpp lines 139-196

The search bar expand/collapse animation is correctly implemented:
- Expand: 0 -> 36px, 200ms, OutCubic
- Collapse: 36 -> 0px, 150ms, InCubic
- Uses QPropertyAnimation on maximumHeight
- Properly restores fixed height after animation

This is the gold standard for how other animations should be implemented. Well done.

### 6.5 Connection breath animation is well implemented

**File**: SerialConfigPanel.cpp lines 238-279

The breathing animation for the status indicator during connection is correctly done:
- QSequentialAnimationGroup with fade-in and fade-out
- 1500ms per half-cycle, InOutSine easing
- Infinite loop
- Properly cleaned up in stopBreathAnimation()

### 6.6 OTA progress animation is well implemented

**File**: OtaWidget.cpp lines 251-268, 387-418

The progress bar uses QPropertyAnimation for smooth value changes, and the completion animation transitions from accent to success color. Both are correct.

---

## 7. Dark/Light Theme Parity (Score: 4/10)

### 7.1 Structural parity is good

All three themes follow the same selector structure. For every `QPushButton#otaStartBtn` in dark_terminal.qss, there is a matching selector in modern_dark.qss and light.qss. This is excellent.

### 7.2 Color value drift between dark themes

The two dark themes (dark_terminal and modern_dark) use completely different color palettes but apply them inconsistently:

| Selector | dark_terminal | modern_dark | Delta |
|----------|---------------|-------------|-------|
| QLabel | `#bac2de` | `#9aa5ce` | Modern is less readable |
| QGroupBox title | `#a6adc8` | `#9aa5ce` | Different values for same semantic role |
| QTreeView hover | `#262637` | `#292e42` | Different hover backgrounds |
| LineEdit color | `#a6e3a1` | `#9ece6a` | Both green, but noticeably different greens |

The two dark themes should be more clearly differentiated in intent. Currently they feel like two developers picked different colors for the same concept.

### 7.3 Light theme uses different structural colors for same semantic role

**File**: light.qss line 610-614

```css
QLabel#connStatus[state="disconnected"] { color: #dc2626; }
QLabel#connStatus[state="error"] { color: #dc2626; }
```

vs dark_terminal.qss line 619-622:

```css
QLabel#connStatus[state="disconnected"] { color: #f38ba8; }
QLabel#connStatus[state="error"] { color: #f38ba8; }
```

The semantic meaning is preserved (both use error/red for disconnected+error), but the hex values are completely different hues. `#dc2626` is a pure red, while `#f38ba8` is a pink-red. This is intentional per-theme adaptation and is actually correct -- the light theme needs a darker red for contrast against white.

### 7.4 Light theme missing QMenu::indicator

**File**: light.qss line 986

```css
QMenu::indicator {
    width: 14px;
    height: 14px;
    margin-left: 6px;
}
```

This selector exists in all three themes, but none of them style the indicator visually (no background, no border, no checkmark). Menu items with checkmarks will show an unstyled 14x14 box.

**Fix**: Add checkmark styling:

```css
QMenu::indicator:checked {
    background-color: #89b4fa;  /* or theme-appropriate accent */
    border: 1px solid #89b4fa;
    border-radius: 2px;
}
QMenu::indicator:unchecked {
    background-color: #181825;
    border: 1px solid #45475a;
    border-radius: 2px;
}
```

---

## 8. Status Indicators (Score: 8/10)

### 8.1 Connection status dot -- excellent

**File**: dark_terminal.qss lines 1026-1043

Four states with distinct colors:
- disconnected: `#6c7086` (muted gray)
- connected: `#a6e3a1` (green)
- connecting: `#f9e2af` (amber)
- error: `#f38ba8` (red)

Plus the breathing animation in SerialConfigPanel.cpp. This is well implemented.

### 8.2 Status indicator is a square, not a circle

**File**: SerialConfigPanel.cpp line 141

```cpp
m_statusIndicator->setFixedSize(8, 8);
```

And dark_terminal.qss line 1030:

```css
border-radius: 4px;
```

At 8x8px with 4px border-radius, this should be a circle. However, QLabel does not clip to border-radius by default in Qt -- the background color will paint as a square behind the rounded border. The visual result is a square with slightly rounded corners, not a clean circle.

**Fix**: Use a custom QWidget with clip or use `border-radius: 4px` plus add `background-clip: content` or draw in paintEvent. Alternatively, increase size to 10x10 for better visibility:

```cpp
m_statusIndicator->setFixedSize(10, 10);
```

```css
QLabel#statusIndicator {
    background-color: #6c7086;
    border: none;
    border-radius: 5px;
}
```

### 8.3 OTA transfer status could be more prominent

**File**: OtaWidget.cpp line 269

```cpp
m_statusLbl->setText(tr("Transferring: %1%").arg(percent));
```

The status label uses the standard QLabel color (#a6adc8 in dark_terminal). During active transfer, this should be more prominent -- perhaps the accent color or the warning color to draw attention.

**Fix**: Add a dynamic property for transfer state:

```cpp
m_statusLbl->setProperty("transferring", true);
m_statusLbl->style()->unpolish(m_statusLbl);
m_statusLbl->style()->polish(m_statusLbl);
```

And in QSS:

```css
QLabel#otaStatusLbl[property="transferring"] {
    color: #89b4fa;  /* accent color */
    font-weight: bold;
}
```

---

## 9. Navigation (Score: 5/10)

### 9.1 No panel transition animation code in reviewed files

None of the five C++ files contain panel switch animation code. The SerialConfigPanel simply appears when shown. The FrameVisualEditor, OtaWidget, QuickCommandBar -- all just show/hide. The CLAUDE.md mandates P0 priority panel slide animations.

This likely exists in NavigationController or PanelManager (not reviewed), but the individual panels do not participate in or support transitions.

### 9.2 QuickCommandBar has no scroll handling for overflow

**File**: QuickCommandBar.cpp lines 93-134

When many commands are added, buttons will overflow the bar horizontally. There is no scroll area, no overflow indicator, and no wrapping. The buttons simply push off-screen and become inaccessible.

**Fix**: Wrap `m_buttonLayout` in a QScrollArea with horizontal scrollbar policy:

```cpp
auto* scrollArea = new QScrollArea;
scrollArea->setWidgetResizable(true);
scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
auto* scrollWidget = new QWidget;
m_buttonLayout = new QHBoxLayout(scrollWidget);
m_buttonLayout->setSpacing(6);
scrollArea->setWidget(scrollWidget);
mainLayout->addWidget(scrollArea);
```

Or limit the number of visible buttons and add a "More..." overflow menu.

### 9.3 FrameVisualEditor has no scroll for long content

**File**: FrameVisualEditor.cpp

The editor has multiple GroupBoxes stacked vertically (header, length, checksum, fields table, preview, apply button). On smaller screens, this will extend below the visible area with no scrollbar. The main layout is a plain QVBoxLayout with no QScrollArea wrapper.

**Fix**: Wrap mainLayout in a QScrollArea:

```cpp
auto* scrollArea = new QScrollArea(this);
scrollArea->setWidgetResizable(true);
scrollArea->setFrameShape(QFrame::NoFrame);
auto* container = new QWidget(scrollArea);
auto* mainLayout = new QVBoxLayout(container);
// ... existing layout code ...
scrollArea->setWidget(container);

auto* outerLayout = new QVBoxLayout(this);
outerLayout->setContentsMargins(0,0,0,0);
outerLayout->addWidget(scrollArea);
```

---

## 10. Overall Polish (Score: 5/10)

### 10.1 QuickCommandBar edit dialog uses bare QDialogButtonBox

**File**: QuickCommandBar.cpp line 181

```cpp
auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
```

The standard QDialogButtonBox renders with platform-native buttons that do not match the themed QPushButton style. On Windows, these buttons will have the system theme (light background, Segoe UI font), not the dark EmbedDebug theme.

**Fix**: Set objectName on the button box and style it, or replace with custom themed QPushButtons:

```cpp
auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
buttons->setObjectName("quickCmdDlgButtonBox");
```

Then in QSS:

```css
QDialogButtonBox QPushButton {
    background-color: #313244;
    color: #cdd6f4;
    border: 1px solid #45475a;
    border-radius: 4px;
    padding: 6px 16px;
    font-size: 13px;
}
```

### 10.2 Close button text is raw "X" instead of an icon

**File**: TerminalSearchBar.cpp line 86

```cpp
m_closeBtn->setText(tr("X"));
```

An "X" character looks crude. It should be a proper icon or at least a styled character. The button is 24x24px with bold font, so the "X" is oversized and harsh.

**Fix**: Use a unicode multiplication sign or times symbol for a softer look:

```cpp
m_closeBtn->setText(QChar(0x00D7));  // multiplication sign
```

Or use an SVG icon:

```cpp
m_closeBtn->setIcon(QIcon(":/icons/close.svg"));
m_closeBtn->setIconSize(QSize(14, 14));
m_closeBtn->setText("");
```

### 10.3 OTA file path uses QLineEdit as a display field

**File**: OtaWidget.cpp line 73-74

```cpp
m_filePathEdit = new QLineEdit;
m_filePathEdit->setObjectName("otaFileLabel");
```

Using a QLineEdit for file path display is functional but the objectName says "otaFileLabel" -- it is confused about whether it is a label or an edit. During transfer, it is disabled but still looks like an editable field. A read-only QLineEdit with disabled styling looks different from a dedicated display widget.

**Fix**: Either rename to `otaFilePathEdit` for clarity, or during transfer, set it to readOnly (not disabled) so it retains the theme styling:

```cpp
m_filePathEdit->setReadOnly(true);  // instead of setEnabled(false)
```

### 10.4 Move up/down buttons in FrameVisualEditor use fixed width

**File**: FrameVisualEditor.cpp line 151, 153

```cpp
moveUpBtn->setFixedWidth(60);
moveDownBtn->setFixedWidth(60);
```

At 60px, the Chinese text "上移" / "下移" (2 characters each) will fit, but the buttons look cramped. The add/remove buttons have no fixed width and stretch. This creates visual inconsistency in the button row.

**Fix**: Remove fixed width and use consistent sizing:

```cpp
moveUpBtn->setMinimumWidth(60);
moveDownBtn->setMinimumWidth(60);
// No setFixedWidth -- let them breathe
```

### 10.5 No empty state styling

When there are no quick commands, the QuickCommandBar shows only the Edit and + buttons with empty space. When there are no fields in the frame editor, the table is an empty rectangle. There are no empty-state illustrations, helpful text, or calls to action.

**Fix**: Add placeholder text when lists are empty:

```cpp
// In QuickCommandBar::rebuildButtons(), when m_commands is empty:
if (m_commands.isEmpty()) {
    auto* hint = new QLabel(tr("Click + to add a quick command"));
    hint->setObjectName("quickCmdEmptyHint");
    // style with text-muted color in QSS
    m_buttonLayout->addWidget(hint);
}
```

### 10.6 OtaWidget log view maximum height is hardcoded

**File**: OtaWidget.cpp line 152

```cpp
m_logView->setMaximumHeight(160);
```

This is a magic number. On high-DPI screens, 160px may be too small for the log text. On standard DPI, it may be fine. The value should scale or be configurable.

---

## Critical Issues Summary (Must Fix)

| # | Severity | File | Line | Issue |
|---|----------|------|------|-------|
| C1 | HIGH | dark_terminal.qss | 135 | QLineEdit color is green globally (should be text-primary, green only for send input) |
| C2 | HIGH | dark_terminal.qss | 506-528 | Missing `connectBtn[state="error"]` QSS rule |
| C3 | HIGH | light.qss | 506-528 | Same missing error state (all themes) |
| C4 | HIGH | FrameVisualEditor.cpp | 173 | Hardcoded font in C++ overrides QSS |
| C5 | MEDIUM | dark_terminal.qss | 143 | QLineEdit:focus uses 1px border (spec says 2px) |
| C6 | MEDIUM | dark_terminal.qss | 279-315 | QComboBox has no :focus state (WCAG 2.4.7 fail) |
| C7 | MEDIUM | QuickCommandBar.cpp | 93-134 | No overflow handling for many commands |
| C8 | MEDIUM | FrameVisualEditor.cpp | 51-228 | No scroll area for long form content |

## Recommended Next Steps

1. Fix C1 (green LineEdit) first -- it affects every text input in the app
2. Fix C2/C3 (error state button) -- error states must be visible
3. Fix C4 (hardcoded font) -- remove the C++ font call, let QSS handle it
4. Add :focus states to ComboBox and SpinBox (C5/C6)
5. Add overflow handling to QuickCommandBar (C7)
6. Add QScrollArea to FrameVisualEditor (C8)
7. Establish and enforce a type scale (eliminate 11px and 14px from UI elements)
8. Implement panel transition animations (P0 in CLAUDE.md)
9. Implement theme switch fade animation (P0 in CLAUDE.md)
10. Review and fix all instances where semantic colors drift between the two dark themes

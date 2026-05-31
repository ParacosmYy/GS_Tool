# ARCH-041: Panel Switching Animation + Breathing Animation Refactor

> Status: Design | Author: System Architect | Date: 2026-06-01

## 1. Panel Switching Animation

### 1.1 Current State

Panels live in a `QVBoxLayout` (not QStackedWidget). `NavigationController::switchToPanel()`
does parallel cross-fade: old panel opacity 1->0 (150ms InCubic) + new panel opacity 0->1
(150ms OutCubic) simultaneously. New panel is `raise()`d on top of old panel during transition.

**Gap vs CLAUDE.md 6.5**: The spec requires "slide in from right + fade in (250ms OutCubic)"
and "slide out left + fade out (200ms InCubic)". Current implementation does fade only, no
positional slide, and at 150ms instead of the spec'd 200/250ms.

### 1.2 Proposed Design

Add horizontal slide to the existing fade. The animation logic stays in NavigationController.
No new class needed -- the logic is small (two parallel QPropertyAnimation groups) and
NavigationController's stated responsibility already covers "panel switching animation".

**Implementation**: Use `QPropertyAnimation` on `QWidget::pos` (geometry.x) combined with
the existing opacity animation, driven by a `QParallelAnimationGroup` for each panel.

```
Old panel:  x: 0 -> -offset (200ms InCubic)  + opacity 1.0 -> 0.0
New panel:  x: +offset -> 0 (250ms OutCubic)  + opacity 0.0 -> 1.0
```

The `offset` should be ~40px (not the full panel width -- a subtle shift, not a page flip).
This matches Linear's panel transitions: perceptible motion without disorienting full slides.

**Concrete change in `switchToPanel()`**:

For the old panel, replace the standalone fade-out with a QParallelAnimationGroup:
- QPropertyAnimation on pos (x: 0 -> -40)
- QPropertyAnimation on opacity (1.0 -> 0.0)

For the new panel, replace `fadeInPanel()` with a QParallelAnimationGroup:
- QPropertyAnimation on pos (x: 40 -> 0)
- QPropertyAnimation on opacity (0.0 -> 1.0)

On animation completion, reset pos to (0,0) and clear the effect, same as current cleanup.

### 1.3 Why Not a Separate PanelAnimator Class

NavigationController already owns the animation logic and has all necessary state
(m_currentPanel, m_panelSwitching, allSwitchablePanels()). Extracting to a new class
would require passing panel lists, current panel state, and completion callbacks across
a boundary -- adding complexity for ~30 lines of animation code. Keep it here until
the animation logic grows beyond 80 lines in a single method.

### 1.4 Nav Indicator Sync

NavIndicatorWidget runs its own 250ms OutCubic slide independently. Both animations
(NavIndicatorWidget slide + panel cross-fade) start at the same time from the same
signal (nav tree click), so they are already synchronized at trigger time. No coupling
needed. If durations diverge later, expose a `kAnimDurationMs` constant in a shared
location (Constants.h) and have both read it.

### 1.5 QStackedWidget Consideration

The current QVBoxLayout + show/hide approach works. QStackedWidget would add an
intermediary layer but provide no animation benefit (it uses the same show/hide
internally). Switching would require touching PanelManager, NavigationController,
and MainWindow setupUI. Risk with no reward. Do not migrate.

---

## 2. Breathing Animation Refactor

### 2.1 Current State (Two Implementations)

| Location | Target | Method | Compliance |
|----------|--------|--------|------------|
| NavigationController | Connection status label (m_connStatusLbl) | QPropertyAnimation on QGraphicsOpacityEffect | Compliant |
| SerialConfigPanel | Status indicator dot (m_statusIndicator) | QTimer at 50ms, manual opacity += 0.05 | Violates CLAUDE.md 6.5 rule 1 |

The NavigationController implementation is correct: 1500ms loop, InOutSine, opacity 0.3-1.0.
The SerialConfigPanel implementation is a manual timer loop that should be replaced.

### 2.2 Refactor Plan

**Replace SerialConfigPanel's QTimer breathing with QPropertyAnimation.**

Changes to `SerialConfigPanel`:

```cpp
// Constructor: remove m_breathTimer creation and timeout lambda
// Add instead:
// m_breathAnim member: QPropertyAnimation* (replaces QTimer*, qreal opacity, bool direction)
```

`setConnecting()`:
```cpp
void SerialConfigPanel::setConnecting()
{
    m_connecting = true;
    updateStatusIndicator("connecting");

    auto* effect = new QGraphicsOpacityEffect(m_statusIndicator);
    effect->setOpacity(1.0);
    m_statusIndicator->setGraphicsEffect(effect);

    m_breathAnim = new QPropertyAnimation(effect, "opacity");
    m_breathAnim->setStartValue(0.3);
    m_breathAnim->setEndValue(1.0);
    m_breathAnim->setDuration(1500);
    m_breathAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_breathAnim->setLoopCount(-1);
    m_breathAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
```

`setConnected()` / `setError()`:
```cpp
// Replace m_breathTimer->stop() with:
if (m_breathAnim) {
    m_breathAnim->stop();
    m_breathAnim = nullptr;
}
// Clean up effect:
if (auto* e = m_statusIndicator->graphicsEffect()) {
    e->setOpacity(1.0);
    m_statusIndicator->setGraphicsEffect(nullptr);
}
```

**Remove from SerialConfigPanel.h**:
- `QTimer* m_breathTimer`
- `qreal m_breathOpacity`
- `bool m_breathIncreasing`

**Add**:
- `QPropertyAnimation* m_breathAnim = nullptr`

### 2.3 Should Breathing Be Extracted to a Reusable Component?

No. The breathing animation is 8 lines of QPropertyAnimation setup. NavigationController
and SerialConfigPanel each target different widgets with slightly different lifecycles.
Extracting to a shared helper would require a class that takes a target widget, manages
its QGraphicsOpacityEffect, handles start/stop lifecycle -- essentially a tiny wrapper
around 2 QPropertyAnimation calls. The abstraction cost exceeds the 16 lines saved.

If a third breathing animation consumer appears, revisit and create a `BreathingAnimation`
utility at that time.

### 2.4 Effect Ownership

When `setConnecting()` creates a `QGraphicsOpacityEffect(effect)` with `m_statusIndicator`
as parent, Qt's parent-child tree owns it. When `setConnected()` calls
`setGraphicsEffect(nullptr)`, Qt deletes the old effect automatically. No manual delete
needed -- same pattern NavigationController already uses.

---

## 3. Impact Analysis

### 3.1 Files Modified

| File | Change | Risk |
|------|--------|------|
| `src/core/NavigationController.cpp` | Replace standalone fade with slide+fade in `switchToPanel()` and `fadeInPanel()` | Medium -- animation timing changes, must test resize edge cases |
| `src/core/NavigationController.h` | No API change (internal refactor only) | Low |
| `src/serial/SerialConfigPanel.h` | Replace QTimer members with QPropertyAnimation* | Low |
| `src/serial/SerialConfigPanel.cpp` | Replace timer-based breathing with QPropertyAnimation in `setConnecting()`, `setConnected()`, `setError()` | Low -- same visual result, better compliance |
| `src/serial/SerialConfigPanel.cpp` constructor | Remove timer setup lambda | Low |

**Files NOT modified**: MainWindow.h/cpp, PanelManager, NavIndicatorWidget, any other panel.

### 3.2 Risk Assessment

**Panel slide animation**: The pos-based animation interacts with layout. If a panel's
position is managed by QVBoxLayout, overriding pos during animation may conflict with
layout updates. Mitigation: call `panel->move()` during animation (layout ignores manual
pos on visible children in a VBoxLayout that uses addWidget). Verify with a resize during
animation -- the layout should snap the panel back, which is acceptable since animation
completes in 250ms and resize during that window is rare.

**Breathing refactor**: Pure mechanical change. The visual output (opacity 0.3-1.0, 1500ms,
InOutSine) matches the current timer output exactly. QPropertyAnimation produces smoother
interpolation than the timer's 0.05 discrete steps (20fps vs Qt's 60fps animation timer).
Zero functional risk.

### 3.3 Performance

- **Slide animation**: Two QParallelAnimationGroups with 4 total animations (2 pos + 2 opacity)
  running for 250ms. Negligible CPU. QGraphicsOpacityEffect triggers software composition --
  already used in current fade-only approach. No regression.
- **Breathing refactor**: Replaces a 50ms timer firing 20x/sec with Qt's animation framework
  using the vsync'd 16ms timer. CPU usage decreases (fewer callbacks, smoother interpolation).
  The QGraphicsOpacityEffect on an 8x8 label is trivial.
- **Effect cleanup**: Both animations remove the QGraphicsOpacityEffect on completion, restoring
  normal widget painting. No persistent overhead.

### 3.4 Compliance Check

| CLAUDE.md 6.5 Rule | Current | After |
|---------------------|---------|-------|
| All animations use QPropertyAnimation | Breathing in SerialConfigPanel uses QTimer | All use QPropertyAnimation |
| Panel slide-in 250ms OutCubic | 150ms fade only (no slide) | 250ms slide+fade OutCubic |
| Panel slide-out 200ms InCubic | 150ms fade only (no slide) | 200ms slide+fade InCubic |
| Breathing 1500ms InOutSine | SerialConfigPanel: 1500ms linear steps | 1500ms InOutSine |
| No animation > 500ms | Compliant | Compliant |
| Performance >= 30fps | Compliant | Improved (smoother) |

---

## 4. Implementation Order

1. **SerialConfigPanel breathing refactor** (low risk, isolated) -- replace QTimer with
   QPropertyAnimation. Verify connecting/disconnecting cycle still works.
2. **NavigationController slide+fade** (medium risk, needs resize testing) -- extend
   existing fade with positional slide. Test panel switching + window resize during animation.
3. **Update CLAUDE.md 6.5 status** -- mark breathing animation and panel slide as implemented.

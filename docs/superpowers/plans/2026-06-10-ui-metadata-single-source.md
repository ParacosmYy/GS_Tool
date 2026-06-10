# UI Metadata Single Source Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Refactor `PanelManager` so panel mapping, panel stack listing, and BasePanel wrapping derive from one internal runtime descriptor table without changing UI behavior.

**Architecture:** Keep the descriptor internal to `src/core/panels` in phase 1 because it references runtime `QWidget*` instances. `NavigationController` keeps receiving `QVector<NavPanelMapping>`, and `MainWindow` keeps consuming the existing `PanelManager` public API.

**Tech Stack:** C++17, Qt Widgets, Qt Test, CMake/Ninja.

---

### Task 1: Add Red Test For Descriptor-Derived Shape

**Files:**
- Create: `tests/test_panel_metadata.cpp`
- Modify: `tests/CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

Create `tests/test_panel_metadata.cpp` with a Qt Test that includes `core/panels/PanelManager.h`, constructs a `PanelManager`, and checks for the planned internal descriptor API through a small test-only accessor only if exposed under a compile definition:

```cpp
#include <QtTest/QtTest>

#include "core/panels/PanelManager.h"

class PanelMetadataTest : public QObject {
    Q_OBJECT

private slots:
    void descriptorApiIsAvailableForPanelManager();
};

void PanelMetadataTest::descriptorApiIsAvailableForPanelManager()
{
#ifdef EMBEDDEBUG_PANEL_DESCRIPTOR_TEST_ACCESS
    PanelManager manager;
    const auto descriptors = manager.panelDescriptorsForTest();
    QVERIFY(descriptors.isEmpty());
#else
    QFAIL("EMBEDDEBUG_PANEL_DESCRIPTOR_TEST_ACCESS is not enabled");
#endif
}

QTEST_MAIN(PanelMetadataTest)
#include "test_panel_metadata.moc"
```

Modify `tests/CMakeLists.txt` to add:

```cmake
add_executable(test_panel_metadata
    test_panel_metadata.cpp
)

target_include_directories(test_panel_metadata PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(test_panel_metadata PRIVATE
    EMBEDDEBUG_PANEL_DESCRIPTOR_TEST_ACCESS
)

target_link_libraries(test_panel_metadata PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Test
)

add_test(NAME PanelMetadata COMMAND test_panel_metadata)
```

- [ ] **Step 2: Run the test to verify RED**

Run: `cmake --build build --target test_panel_metadata`

Expected: FAIL because `PanelManager::panelDescriptorsForTest()` does not exist.

### Task 2: Add Internal Descriptor API

**Files:**
- Modify: `src/core/panels/PanelManager.h`
- Modify: `src/core/panels/PanelManagerQuery.cpp`

- [ ] **Step 1: Add the minimal descriptor type**

Add private internal types to `PanelManager`:

```cpp
enum class PanelWrapperPolicy {
    Wrapped,
    RawPersistent,
    Overlay,
    FixedBar,
    Floating
};

struct PanelDescriptor {
    const char* groupKey = "";
    const char* titleKey = "";
    QWidget* rawPanel = nullptr;
    PanelWrapperPolicy wrapperPolicy = PanelWrapperPolicy::Wrapped;
    bool navVisible = true;
    bool includeInPanelStack = true;
};

QVector<PanelDescriptor> panelDescriptors() const;
```

Add test access under the compile definition:

```cpp
#ifdef EMBEDDEBUG_PANEL_DESCRIPTOR_TEST_ACCESS
public:
    QVector<PanelDescriptor> panelDescriptorsForTest() const;
private:
#endif
```

- [ ] **Step 2: Implement the minimal API**

In `PanelManagerQuery.cpp`, implement:

```cpp
#ifdef EMBEDDEBUG_PANEL_DESCRIPTOR_TEST_ACCESS
QVector<PanelManager::PanelDescriptor> PanelManager::panelDescriptorsForTest() const
{
    return panelDescriptors();
}
#endif
```

Initially return an empty vector from `panelDescriptors()` to pass Task 1.

- [ ] **Step 3: Verify GREEN**

Run: `cmake --build build --target test_panel_metadata`

Expected: PASS.

### Task 3: Populate Descriptors And Derive `panelMappings()`

**Files:**
- Modify: `src/core/panels/PanelManagerQuery.cpp`

- [ ] **Step 1: Add a failing count/order test**

Extend `test_panel_metadata.cpp` after constructing the manager without created panels:

```cpp
QCOMPARE(descriptors.size(), 43);
QCOMPARE(QString::fromUtf8(descriptors.first().groupKey), QStringLiteral("连接"));
QCOMPARE(QString::fromUtf8(descriptors.first().titleKey), QStringLiteral("配置"));
```

Run: `cmake --build build --target test_panel_metadata`

Expected: FAIL because descriptors are empty.

- [ ] **Step 2: Fill `panelDescriptors()` in current `panelMappings()` order**

Return a descriptor list matching the current navigation order and using `RawPersistent` for `m_terminal`, `Overlay` for `m_searchBar`, and `FixedBar` for `m_quickCmdBar`.

- [ ] **Step 3: Rewrite `panelMappings()` from descriptors**

For each descriptor with `navVisible`, map `Wrapped` to `wrapper(rawPanel)` and all other visible policies to `rawPanel`.

- [ ] **Step 4: Verify**

Run: `cmake --build build --target test_panel_metadata`

Expected: PASS.

### Task 4: Derive `allPanels()` And `wrapPanels()`

**Files:**
- Modify: `src/core/panels/PanelManagerQuery.cpp`
- Modify: `src/core/panels/PanelManagerWrap.cpp`
- Modify: `src/core/panels/PanelManagerPanels.h`

- [ ] **Step 1: Add behavior-preserving assertions**

Add tests that descriptor policies contain exactly one terminal raw persistent entry and that search/quick command entries are present with non-wrapped policies.

- [ ] **Step 2: Derive `allPanels()`**

Iterate descriptors with `includeInPanelStack == true` and return `wrapper(rawPanel)` for wrapped panels, otherwise `rawPanel`.

- [ ] **Step 3: Derive `wrapPanels()`**

Iterate descriptors with `PanelWrapperPolicy::Wrapped`, keep current null skipping, `setVisible(false)`, `setObjectName(panel->objectName() + "Wrapper")`, `m_wrappers[panel] = wrapper`, and statistic increments.

- [ ] **Step 4: Update registration comment**

In `PanelManagerPanels.h`, update the "新增面板时必须" comment to mention descriptor registration instead of three repeated lists.

- [ ] **Step 5: Verify**

Run: `cmake --build build --target test_panel_metadata`

Expected: PASS.

### Task 5: Full Verification

**Files:**
- No edits unless verification exposes a defect.

- [ ] **Step 1: Build**

Run: `cmake --build build`

Expected: exit code 0.

- [ ] **Step 2: Run tests**

Run: `Push-Location build; ctest --output-on-failure; Pop-Location`

Expected: all registered tests pass.

- [ ] **Step 3: Inspect diff**

Run: `git diff -- docs/prd/PRD_070_UI_Metadata_Single_Source.md docs/superpowers/plans/2026-06-10-ui-metadata-single-source.md tests/test_panel_metadata.cpp tests/CMakeLists.txt src/core/panels/PanelManager.h src/core/panels/PanelManagerQuery.cpp src/core/panels/PanelManagerWrap.cpp src/core/panels/PanelManagerPanels.h`

Expected: only PRD, plan, test, and focused panel manager changes.

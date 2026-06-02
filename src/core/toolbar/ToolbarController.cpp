/**
 * @file ToolbarController.cpp
 * @brief 工具栏控制器实现 - 主工具栏控件的创建、布局和事件转发
 */

#include "core/toolbar/ToolbarController.h"
#include "core/recording/RecordingController.h"
#include "core/theme/ThemeManager.h"
#include "core/theme/Constants.h"

#include <QMainWindow>
#include <QToolBar>
#include <QComboBox>
#include <QAction>
#include <QLabel>

/**
 * @brief 构造工具栏控制器
 * @param recordingController 录制控制器，用于在工具栏中添加录制/回放按钮
 * @param parent 父对象
 */
ToolbarController::ToolbarController(RecordingController* recordingController, QObject* parent)
    : QObject(parent)
    , m_recordingController(recordingController)
    , m_toolbar(nullptr)
    , m_displayModeCombo(nullptr)
    , m_layoutCombo(nullptr)
    , m_themeCombo(nullptr)
    , m_langCombo(nullptr)
    , m_timestampAction(nullptr)
    , m_dirPrefixAction(nullptr)
    , m_clearAction(nullptr)
    , m_exportAction(nullptr)
    , m_bgAction(nullptr)
{
}

/** @brief 获取工具栏指针 */
QToolBar* ToolbarController::toolbar() const
{
    return m_toolbar;
}

/**
 * @brief 创建并返回工具栏，添加到主窗口
 *
 * 控件布局:
 * [显示模式] [终端布局] 时间戳 [TX/RX] 清屏 | 导出 背景 | 录制 停止录制 回放日志 停止回放 | 主题: [下拉] 语言: [下拉]
 *
 * @param parent 主窗口实例
 * @return 创建的工具栏指针
 */
QToolBar* ToolbarController::createToolbar(QMainWindow* parent)
{
    m_toolbar = parent->addToolBar(tr("主工具栏"));
    m_toolbar->setObjectName("mainToolbar");
    m_toolbar->setMovable(false);     // 禁止拖拽移动
    m_toolbar->setFloatable(false);   // 禁止浮动

    createDisplayModeGroup(m_toolbar);
    m_toolbar->addSeparator();
    createConnectionGroup(m_toolbar);

    // ---- 连接内部信号转发 ----
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ToolbarController::displayModeChanged);
    connect(m_layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ToolbarController::terminalLayoutChanged);
    connect(m_timestampAction, &QAction::toggled,
            this, &ToolbarController::timestampToggled);
    connect(m_dirPrefixAction, &QAction::toggled,
            this, &ToolbarController::dirPrefixToggled);
    connect(m_clearAction, &QAction::triggered,
            this, &ToolbarController::clearRequested);
    connect(m_exportAction, &QAction::triggered,
            this, &ToolbarController::exportRequested);
    connect(m_bgAction, &QAction::triggered,
            this, &ToolbarController::bgSettingsRequested);
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ToolbarController::themeChanged);
    connect(m_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ToolbarController::languageChanged);

    // 用 ThemeManager 当前可用主题初始化下拉框
    setAvailableThemes(ThemeManager::instance().availableThemes());

    return m_toolbar;
}

/**
 * @brief 创建显示模式相关控件组
 *
 * 包含: 显示模式下拉框(文本/HEX/混合/十进制)、终端布局下拉框(混合/左右分栏/上下分栏)、
 * 时间戳开关、方向前缀[TX/RX]开关、清屏按钮
 *
 * @param toolbar 目标工具栏
 */
void ToolbarController::createDisplayModeGroup(QToolBar* toolbar)
{
    // 显示模式下拉框: 文本/HEX/混合/十进制
    m_displayModeCombo = new QComboBox(toolbar->parentWidget());
    m_displayModeCombo->setObjectName("displayModeCombo");
    m_displayModeCombo->addItems({tr("文本"), tr("HEX"), tr("混合"), tr("十进制")});
    m_displayModeCombo->setFixedWidth(Layout::kComboFixedWidth);
    toolbar->addWidget(m_displayModeCombo);

    // 终端布局模式下拉框: 混合/左右分栏/上下分栏
    m_layoutCombo = new QComboBox(toolbar->parentWidget());
    m_layoutCombo->setObjectName("layoutCombo");
    m_layoutCombo->addItems({tr("混合"), tr("左右分栏"), tr("上下分栏")});
    m_layoutCombo->setFixedWidth(Layout::kComboFixedWidth);
    m_layoutCombo->setToolTip(tr("终端布局: 混合显示或TX/RX分栏"));
    toolbar->addWidget(m_layoutCombo);

    // 时间戳开关（可切换 Action）
    m_timestampAction = toolbar->addAction(tr("时间戳"));
    m_timestampAction->setObjectName("timestampAction");
    m_timestampAction->setCheckable(true);
    m_timestampAction->setChecked(false);

    // 收发方向前缀开关（可切换 Action）
    m_dirPrefixAction = toolbar->addAction(tr("[TX/RX]"));
    m_dirPrefixAction->setObjectName("dirPrefixAction");
    m_dirPrefixAction->setCheckable(true);
    m_dirPrefixAction->setChecked(false);
    m_dirPrefixAction->setToolTip(tr("显示收发方向前缀"));

    // 清屏按钮
    m_clearAction = toolbar->addAction(tr("清屏"));
    m_clearAction->setObjectName("clearAction");
}

/**
 * @brief 创建连接相关控制组
 *
 * 包含: 导出按钮、背景设置按钮、录制/回放(委托RecordingController)、
 * 主题切换下拉框、语言切换下拉框
 *
 * @param toolbar 目标工具栏
 */
void ToolbarController::createConnectionGroup(QToolBar* toolbar)
{
    // 导出按钮
    m_exportAction = toolbar->addAction(tr("导出"));
    m_exportAction->setObjectName("exportAction");

    // 背景设置按钮
    m_bgAction = toolbar->addAction(tr("背景"));
    m_bgAction->setObjectName("bgSettingsAction");
    m_bgAction->setToolTip(tr("背景图设置: 磨砂/透明度/特效"));

    toolbar->addSeparator();

    // 日志录制/回放按钮（委托给 RecordingController 创建和管理）
    m_recordingController->setupActions(toolbar);

    toolbar->addSeparator();

    // 主题切换下拉框
    auto* themeLabel = new QLabel(tr(" 主题: "));
    themeLabel->setObjectName("themeLabel");
    toolbar->addWidget(themeLabel);

    m_themeCombo = new QComboBox(toolbar->parentWidget());
    m_themeCombo->setObjectName("themeCombo");
    m_themeCombo->setFixedWidth(130);
    toolbar->addWidget(m_themeCombo);

    // 语言切换下拉框
    auto* langLabel = new QLabel(tr(" 语言: "));
    langLabel->setObjectName("langLabel");
    toolbar->addWidget(langLabel);

    m_langCombo = new QComboBox(toolbar->parentWidget());
    m_langCombo->setObjectName("langCombo");
    m_langCombo->addItem(QStringLiteral("中文"), Language::CHINESE);
    m_langCombo->addItem(QStringLiteral("English"), Language::ENGLISH);
    m_langCombo->setFixedWidth(Layout::kComboFixedWidth);
    toolbar->addWidget(m_langCombo);
}

/**
 * @brief 设置可用主题列表
 * 将内部名称（如 dark_terminal）转换为友好显示名称（如 Dark Terminal）
 * 使用 blockSignals 防止填充过程中触发 themeChanged 信号
 * @param themes 主题名称列表
 */
void ToolbarController::setAvailableThemes(const QStringList& themes)
{
    if (!m_themeCombo) return;

    m_themeCombo->blockSignals(true);  // 填充过程中不触发信号
    m_themeCombo->clear();

    for (const QString& name : themes) {
        // 显示友好名称: dark_terminal -> Dark Terminal
        QString display = name;
        display[0] = display[0].toUpper();
        // 将下划线替换为空格并大写每个单词首字母
        QStringList parts = display.split('_');
        for (auto& part : parts) {
            if (!part.isEmpty()) part[0] = part[0].toUpper();
        }
        // itemData 存储原始名称，显示转换后的友好名称
        m_themeCombo->addItem(parts.join(" "), name);
    }

    m_themeCombo->blockSignals(false);
}

/**
 * @brief 设置当前选中的主题
 * 通过 itemData 中存储的原始主题名称匹配
 * @param themeName 主题名称
 */
void ToolbarController::setCurrentTheme(const QString& themeName)
{
    if (!m_themeCombo) return;

    for (int i = 0; i < m_themeCombo->count(); ++i) {
        if (m_themeCombo->itemData(i).toString() == themeName) {
            m_themeCombo->setCurrentIndex(i);
            break;
        }
    }
}

/**
 * @brief 根据索引获取主题名称
 * @param index 下拉框索引
 * @return 主题原始名称，索引无效时返回空字符串
 */
QString ToolbarController::themeNameAt(int index) const
{
    if (!m_themeCombo || index < 0 || index >= m_themeCombo->count()) return {};
    return m_themeCombo->itemData(index).toString();
}

/**
 * @brief 设置当前选中的语言
 * @param langCode 语言代码
 */
void ToolbarController::setCurrentLanguage(const QString& langCode)
{
    if (!m_langCombo) return;

    for (int i = 0; i < m_langCombo->count(); ++i) {
        if (m_langCombo->itemData(i).toString() == langCode) {
            m_langCombo->setCurrentIndex(i);
            break;
        }
    }
}

/**
 * @brief 根据索引获取语言代码
 * @param index 下拉框索引
 * @return 语言代码，索引无效时返回空字符串
 */
QString ToolbarController::languageCodeAt(int index) const
{
    if (!m_langCombo || index < 0 || index >= m_langCombo->count()) return {};
    return m_langCombo->itemData(index).toString();
}

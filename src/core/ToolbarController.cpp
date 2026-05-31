#include "ToolbarController.h"
#include "RecordingController.h"
#include "ThemeManager.h"
#include "Constants.h"

#include <QMainWindow>
#include <QToolBar>
#include <QComboBox>
#include <QAction>
#include <QLabel>

ToolbarController::ToolbarController(RecordingController* recordingController, QObject* parent)
    : QObject(parent)
    , m_recordingController(recordingController)
    , m_toolbar(nullptr)
    , m_displayModeCombo(nullptr)
    , m_themeCombo(nullptr)
    , m_langCombo(nullptr)
    , m_timestampAction(nullptr)
    , m_dirPrefixAction(nullptr)
    , m_clearAction(nullptr)
    , m_exportAction(nullptr)
    , m_bgAction(nullptr)
{
}

QToolBar* ToolbarController::toolbar() const
{
    return m_toolbar;
}

QToolBar* ToolbarController::createToolbar(QMainWindow* parent)
{
    m_toolbar = parent->addToolBar(tr("主工具栏"));
    m_toolbar->setObjectName("mainToolbar");
    m_toolbar->setMovable(false);
    m_toolbar->setFloatable(false);

    // 显示模式下拉框
    m_displayModeCombo = new QComboBox;
    m_displayModeCombo->setObjectName("displayModeCombo");
    m_displayModeCombo->addItems({tr("文本"), tr("HEX"), tr("混合"), tr("十进制")});
    m_displayModeCombo->setFixedWidth(80);
    m_toolbar->addWidget(m_displayModeCombo);

    // 时间戳开关
    m_timestampAction = m_toolbar->addAction(tr("时间戳"));
    m_timestampAction->setObjectName("timestampAction");
    m_timestampAction->setCheckable(true);
    m_timestampAction->setChecked(false);

    // 收发方向前缀开关
    m_dirPrefixAction = m_toolbar->addAction(tr("[TX/RX]"));
    m_dirPrefixAction->setObjectName("dirPrefixAction");
    m_dirPrefixAction->setCheckable(true);
    m_dirPrefixAction->setChecked(false);
    m_dirPrefixAction->setToolTip(tr("显示收发方向前缀"));

    // 清屏按钮
    m_clearAction = m_toolbar->addAction(tr("清屏"));
    m_clearAction->setObjectName("clearAction");

    m_toolbar->addSeparator();

    // 导出按钮
    m_exportAction = m_toolbar->addAction(tr("导出"));
    m_exportAction->setObjectName("exportAction");

    // 背景设置按钮
    m_bgAction = m_toolbar->addAction(tr("背景"));
    m_bgAction->setObjectName("bgSettingsAction");
    m_bgAction->setToolTip(tr("背景图设置: 磨砂/透明度/特效"));

    m_toolbar->addSeparator();

    // 日志录制/回放按钮（委托给RecordingController管理）
    m_recordingController->setupActions(m_toolbar);

    m_toolbar->addSeparator();

    // 主题切换下拉框
    auto* themeLabel = new QLabel(tr(" 主题: "));
    themeLabel->setObjectName("themeLabel");
    m_toolbar->addWidget(themeLabel);

    m_themeCombo = new QComboBox;
    m_themeCombo->setObjectName("themeCombo");
    m_themeCombo->setFixedWidth(130);
    m_toolbar->addWidget(m_themeCombo);

    // 语言切换下拉框
    auto* langLabel = new QLabel(tr(" 语言: "));
    langLabel->setObjectName("langLabel");
    m_toolbar->addWidget(langLabel);

    m_langCombo = new QComboBox;
    m_langCombo->setObjectName("langCombo");
    m_langCombo->addItem(QStringLiteral("中文"), Language::CHINESE);
    m_langCombo->addItem(QStringLiteral("English"), Language::ENGLISH);
    m_langCombo->setFixedWidth(90);
    m_toolbar->addWidget(m_langCombo);

    // 连接内部信号转发
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ToolbarController::displayModeChanged);
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

    // 用ThemeManager当前可用主题初始化下拉框
    setAvailableThemes(ThemeManager::instance().availableThemes());

    return m_toolbar;
}

void ToolbarController::setAvailableThemes(const QStringList& themes)
{
    if (!m_themeCombo) return;

    m_themeCombo->blockSignals(true);
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
        m_themeCombo->addItem(parts.join(" "), name);
    }

    m_themeCombo->blockSignals(false);
}

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

QString ToolbarController::themeNameAt(int index) const
{
    if (!m_themeCombo || index < 0 || index >= m_themeCombo->count()) return {};
    return m_themeCombo->itemData(index).toString();
}

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

QString ToolbarController::languageCodeAt(int index) const
{
    if (!m_langCombo || index < 0 || index >= m_langCombo->count()) return {};
    return m_langCombo->itemData(index).toString();
}

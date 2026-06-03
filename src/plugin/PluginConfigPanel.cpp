/**
 * @file PluginConfigPanel.cpp
 * @brief 插件配置面板实现 — 插件管理 UI
 *
 * 提供"加载 / 卸载 / 扫描"三个操作按钮，
 * 按钮交互通过信号委托给 PluginManager 执行。
 */

#include "plugin/PluginConfigPanel.h"
#include "plugin/PluginManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QFileInfo>

/**
 * @brief 构造函数
 *
 * 初始化 UI 布局，创建控件并连接信号。
 *
 * @param parent 父控件
 */
PluginConfigPanel::PluginConfigPanel(QWidget* parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_pluginList(nullptr)
    , m_loadBtn(nullptr)
    , m_unloadBtn(nullptr)
    , m_scanBtn(nullptr)
    , m_manager(nullptr)
{
    setObjectName(QStringLiteral("PluginConfigPanel"));
    setupUI();
}

/**
 * @brief 绑定插件管理器
 *
 * 保存指针并连接插件加载/卸载信号，以便自动刷新列表。
 *
 * @param manager PluginManager 实例
 */
void PluginConfigPanel::setPluginManager(PluginManager* manager)
{
    if (m_manager) {
        /* 断开旧连接 */
        disconnect(m_manager, &PluginManager::pluginLoaded,
                   this, nullptr);
        disconnect(m_manager, &PluginManager::pluginUnloaded,
                   this, nullptr);
    }

    m_manager = manager;

    if (m_manager) {
        connect(m_manager, &PluginManager::pluginLoaded,
                this, [this]() { refreshList(); });
        connect(m_manager, &PluginManager::pluginUnloaded,
                this, [this]() { refreshList(); });
        refreshList();
    }
}

/**
 * @brief 初始化 UI 布局和控件
 *
 * 布局结构:
 * @code
 * ┌─────────────────────────┐
 * │     Plugin Manager      │  ← 标题
 * ├─────────────────────────┤
 * │                         │
 * │      Plugin List        │  ← QListWidget
 * │                         │
 * ├─────────────────────────┤
 * │ [Load...] [Unload] [Scan]│  ← 按钮行
 * └─────────────────────────┘
 * @endcode
 */
void PluginConfigPanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    /* 标题 */
    m_titleLabel = new QLabel(tr("插件管理器"), this);
    m_titleLabel->setObjectName(QStringLiteral("pluginManagerTitle"));
    mainLayout->addWidget(m_titleLabel);

    /* 插件列表 */
    m_pluginList = new QListWidget(this);
    m_pluginList->setObjectName(QStringLiteral("pluginList"));
    mainLayout->addWidget(m_pluginList, 1);

    /* 按钮行 */
    auto* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);

    m_loadBtn = new QPushButton(tr("加载..."), this);
    m_loadBtn->setObjectName(QStringLiteral("loadPluginBtn"));

    m_unloadBtn = new QPushButton(tr("卸载"), this);
    m_unloadBtn->setObjectName(QStringLiteral("unloadPluginBtn"));

    m_scanBtn = new QPushButton(tr("扫描"), this);
    m_scanBtn->setObjectName(QStringLiteral("scanPluginBtn"));

    btnLayout->addWidget(m_loadBtn);
    btnLayout->addWidget(m_unloadBtn);
    btnLayout->addWidget(m_scanBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    /* ── 信号连接 ── */

    /* 加载按钮: 弹出文件选择对话框 */
    connect(m_loadBtn, &QPushButton::clicked, this, [this]() {
#ifdef Q_OS_WIN
        const QString filter = tr("Plugins (*.dll)");
#else
        const QString filter = tr("Plugins (*.so)");
#endif
        const QString path = QFileDialog::getOpenFileName(
            this, tr("Select Plugin"), QString(), filter);
        if (!path.isEmpty()) {
            emit loadPluginRequested(path);
        }
    });

    /* 卸载按钮: 取当前选中项 */
    connect(m_unloadBtn, &QPushButton::clicked, this, [this]() {
        QListWidgetItem* item = m_pluginList->currentItem();
        if (!item) {
            return;
        }
        const QString name = item->text();
        emit unloadPluginRequested(name);
    });

    /* 扫描按钮: 通过 PluginManager 扫描默认目录 */
    connect(m_scanBtn, &QPushButton::clicked, this, [this]() {
        if (!m_manager) {
            return;
        }
#ifdef Q_OS_WIN
        const QString defaultDir = QApplication::applicationDirPath()
                                   + QStringLiteral("/plugins");
#else
        const QString defaultDir = QApplication::applicationDirPath()
                                   + QStringLiteral("/../lib/plugins");
#endif
        const QStringList found = m_manager->scanPlugins(defaultDir);
        /* 将扫描结果加入列表（去重） */
        for (const QString& path : found) {
            const QFileInfo fi(path);
            const QString baseName = fi.completeBaseName();
            /* 避免重复添加 */
            bool exists = false;
            for (int i = 0; i < m_pluginList->count(); ++i) {
                if (m_pluginList->item(i)->data(Qt::UserRole).toString()
                    == path) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                auto* listItem = new QListWidgetItem(
                    fi.fileName() + tr(" (discovered)"), m_pluginList);
                listItem->setData(Qt::UserRole, path);
            }
        }
    });
}

/**
 * @brief 从 PluginManager 刷新插件列表显示
 *
 * 清空列表后，遍历已加载插件名称重新填充。
 */
void PluginConfigPanel::refreshList()
{
    if (!m_manager) {
        return;
    }

    m_pluginList->clear();
    const QStringList names = m_manager->loadedPluginNames();
    for (const QString& name : names) {
        auto* item = new QListWidgetItem(
            name + tr(" (loaded)"), m_pluginList);
        item->setData(Qt::UserRole, name);
    }
}

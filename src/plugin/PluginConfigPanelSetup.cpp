/**
 * @file PluginConfigPanelSetup.cpp
 * @brief 插件配置面板 — UI初始化与信号连接实现
 *
 * 从 PluginConfigPanel.cpp 拆分而来，包含:
 *   - setupUI(): 构建面板布局(标题/列表/详情/按钮)并连接所有信号
 *   - 加载/卸载/扫描按钮的点击处理逻辑
 *   - 列表选择变化的详情面板联动
 *
 * 列表刷新和详情面板更新见 PluginConfigPanelUI.cpp。
 */

#include "plugin/PluginConfigPanel.h"
#include "plugin/PluginManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QApplication>
#include <QFileInfo>

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

    /* 详情面板 */
    auto* detailGroup = new QWidget(this);
    detailGroup->setObjectName(QStringLiteral("pluginDetailGroup"));
    auto* detailLayout = new QVBoxLayout(detailGroup);
    detailLayout->setContentsMargins(4, 4, 4, 4);
    detailLayout->setSpacing(4);

    m_detailNameLabel = new QLabel(tr("名称：-"), detailGroup);
    m_detailNameLabel->setObjectName(QStringLiteral("pluginDetailName"));

    m_detailVersionLabel = new QLabel(tr("版本：-"), detailGroup);
    m_detailVersionLabel->setObjectName(QStringLiteral("pluginDetailVersion"));

    m_detailStatusLabel = new QLabel(tr("状态：-"), detailGroup);
    m_detailStatusLabel->setObjectName(QStringLiteral("pluginDetailStatus"));

    m_detailDescEdit = new QTextEdit(detailGroup);
    m_detailDescEdit->setObjectName(QStringLiteral("pluginDetailDesc"));
    m_detailDescEdit->setReadOnly(true);
    m_detailDescEdit->setMaximumHeight(80);
    m_detailDescEdit->setPlaceholderText(tr("插件描述信息"));

    detailLayout->addWidget(m_detailNameLabel);
    detailLayout->addWidget(m_detailVersionLabel);
    detailLayout->addWidget(m_detailStatusLabel);
    detailLayout->addWidget(m_detailDescEdit);

    mainLayout->addWidget(detailGroup);

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
        const QString filter = tr("插件 (*.dll)");
#else
        const QString filter = tr("插件 (*.so)");
#endif
        const QString path = QFileDialog::getOpenFileName(
            this, tr("选择插件"), QString(), filter);
        if (!path.isEmpty()) {
            ++m_totalPluginLoads;
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
        ++m_totalPluginUnloads;
        emit unloadPluginRequested(name);
    });

    /* 扫描按钮: 通过 PluginManager 扫描默认目录 */
    connect(m_scanBtn, &QPushButton::clicked, this, [this]() {
        if (!m_manager) {
            return;
        }
        ++m_totalScans;
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

    /* 列表选择变化: 更新详情面板 */
    connect(m_pluginList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem* /*previous*/) {
                if (!current || !m_manager) {
                    m_detailNameLabel->setText(tr("名称：-"));
                    m_detailVersionLabel->setText(tr("版本：-"));
                    m_detailStatusLabel->setText(tr("状态：-"));
                    m_detailDescEdit->clear();
                    return;
                }
                const QString name = current->data(Qt::UserRole).toString();
                if (m_manager->isPluginLoaded(name)) {
                    updateDetailPanel(name);
                } else {
                    /* 扫描到的未加载插件 */
                    m_detailNameLabel->setText(tr("名称：%1").arg(
                        QFileInfo(name).fileName()));
                    m_detailVersionLabel->setText(tr("版本：未知"));
                    m_detailStatusLabel->setText(tr("状态：未加载"));
                    m_detailDescEdit->clear();
                }
            });
}

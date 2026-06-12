/**
 * @file SvdViewerWidget.cpp
 * @brief SVD寄存器查看器主控件实现 — 构造/UI布局/文件加载/搜索/选中处理
 *
 * 包含构造函数、setupUi/setupConnections、loadSvdFile、搜索/过滤、
 * 树选中回调、详情面板更新、外设过滤重建。
 * 统计getter和resetStatistics在 SvdViewerWidgetStats.cpp。
 */

#include "protocol/svd/SvdViewerWidget.h"
#include "protocol/svd/SvdRegisterTreeModel.h"
#include "protocol/svd/SvdBitFieldWidget.h"
#include "protocol/svd/SvdTypes.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QRegularExpression>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造SVD查看器主控件，初始化UI和模型 @param parent 父控件 */
SvdViewerWidget::SvdViewerWidget(QWidget* parent)
    : QWidget(parent)
    , m_treeModel(new SvdRegisterTreeModel(this))
{
    setupUi();
    setupConnections();
}

// ============================================================================
// UI初始化
// ============================================================================

/** @brief 初始化UI布局: 顶部工具栏 + 左树 + 右详情(信息+位字段图) */
void SvdViewerWidget::setupUi()
{
    setObjectName("SvdViewerWidget");

    /* ---- 工具栏 ---- */
    m_openBtn = new QPushButton(tr("打开SVD"), this);
    m_openBtn->setObjectName("btnSvdOpenFile");

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("svdSearchEdit");
    m_searchEdit->setPlaceholderText(tr("搜索寄存器/字段..."));
    m_searchEdit->setClearButtonEnabled(true);

    m_filterCombo = new QComboBox(this);
    m_filterCombo->setObjectName("cmbSvdFilter");
    m_filterCombo->addItem(tr("全部外设"));

    m_fileLabel = new QLabel(tr("未加载SVD文件"), this);
    m_fileLabel->setObjectName("lblSvdFileName");

    m_statusLabel = new QLabel(tr("就绪"), this);
    m_statusLabel->setObjectName("lblSvdStatus");

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(m_openBtn);
    toolbar->addWidget(m_searchEdit, 1);
    toolbar->addWidget(m_filterCombo);
    toolbar->addStretch();
    toolbar->addWidget(m_fileLabel, 1);
    toolbar->addWidget(m_statusLabel);

    /* ---- 左侧: 寄存器树 ---- */
    m_treeView = new QTreeView(this);
    m_treeView->setObjectName("svdRegisterTree");
    m_treeView->setModel(m_treeModel);
    m_treeView->setAlternatingRowColors(true);
    m_treeView->setSortingEnabled(false);
    m_treeView->header()->setSectionResizeMode(
        SvdRegisterTreeModel::ColName, QHeaderView::Stretch);
    m_treeView->header()->setSectionResizeMode(
        SvdRegisterTreeModel::ColAddress, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(
        SvdRegisterTreeModel::ColAccess, QHeaderView::ResizeToContents);
    m_treeView->header()->setSectionResizeMode(
        SvdRegisterTreeModel::ColResetValue, QHeaderView::ResizeToContents);
    m_treeView->header()->setStretchLastSection(true);

    /* ---- 右侧: 详情面板 ---- */
    m_detailView = new QTextEdit(this);
    m_detailView->setObjectName("svdDetailView");
    m_detailView->setReadOnly(true);
    m_detailView->setPlaceholderText(tr("选择寄存器查看详情"));

    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    m_detailView->setFont(monoFont);

    m_bitFieldWidget = new SvdBitFieldWidget(this);
    m_bitFieldWidget->setObjectName("svdBitFieldWidget");

    auto* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_detailView, 3);
    rightLayout->addWidget(m_bitFieldWidget, 1);

    auto* rightPanel = new QWidget(this);
    rightPanel->setObjectName("svdRightPanel");
    rightPanel->setLayout(rightLayout);

    /* ---- 主分割器: 左树 + 右详情 ---- */
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("svdMainSplitter");
    splitter->addWidget(m_treeView);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    /* ---- 主布局 ---- */
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(toolbar);
    mainLayout->addWidget(splitter, 1);
}

/** @brief 初始化信号/槽连接 */
void SvdViewerWidget::setupConnections()
{
    connect(m_openBtn, &QPushButton::clicked,
            this, &SvdViewerWidget::onOpenFileClicked);

    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &SvdViewerWidget::onSearchTextChanged);

    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SvdViewerWidget::onFilterChanged);

    connect(m_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SvdViewerWidget::onTreeSelectionChanged);

    connect(m_treeView, &QTreeView::expanded,
            this, &SvdViewerWidget::onTreeExpanded);

    connect(m_treeView, &QTreeView::collapsed,
            this, &SvdViewerWidget::onTreeCollapsed);

    connect(m_bitFieldWidget, &SvdBitFieldWidget::fieldClicked,
            this, [this](int index) {
        Q_UNUSED(index)
        /* 字段点击可扩展为发射fieldSelected信号 */
    });
}

// ============================================================================
// 文件加载
// ============================================================================

/** @brief 加载SVD文件并更新树模型 @param filePath SVD文件路径 */
void SvdViewerWidget::loadSvdFile(const QString& filePath)
{
    if (filePath.isEmpty()) { return; }

    ++m_totalFileLoads;

    /* TODO: 当SvdParser(另一Agent实现)就绪后，替换此处为实际解析 */
    /* SvdParser parser; */
    /* SvdDevice device = parser.parse(filePath); */
    /* m_treeModel->setDevice(device); */

    /* 占位: 清除旧数据并更新状态 */
    m_treeModel->clear();
    m_currentFilePath = filePath;
    m_fileLabel->setText(filePath);

    QFileInfo fi(filePath);
    m_statusLabel->setText(tr("已加载: %1").arg(fi.fileName()));

    emit svdFileLoaded(fi.fileName());

    rebuildFilterCombo();
}

/** @brief 获取当前加载的SVD文件路径 @return 文件路径 */
QString SvdViewerWidget::currentFilePath() const
{
    return m_currentFilePath;
}

// ============================================================================
// 槽函数
// ============================================================================

/** @brief 打开文件按钮回调 — 弹出文件选择对话框 */
void SvdViewerWidget::onOpenFileClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, tr("打开SVD文件"), QString(),
        tr("SVD文件 (*.svd);;XML文件 (*.xml);;所有文件 (*)"));

    if (!path.isEmpty()) {
        loadSvdFile(path);
    }
}

/** @brief 搜索文本变化回调 — 高亮匹配项并展开对应节点 @param text 搜索关键词 */
void SvdViewerWidget::onSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    ++m_totalSearchCount;

    /* TODO: 实现搜索逻辑 — 遍历模型匹配名称/描述，展开并选中 */
    m_statusLabel->setText(tr("搜索: \"%1\"").arg(text));
}

/** @brief 外设过滤器变化回调 — 按外设过滤显示 @param index 选中索引 */
void SvdViewerWidget::onFilterChanged(int index)
{
    Q_UNUSED(index)

    /* TODO: 实现过滤逻辑 — 隐藏非选中外设的顶层节点 */
    QString filter = m_filterCombo->currentText();
    m_statusLabel->setText(tr("过滤: %1").arg(filter));
}

/** @brief 树选中项变化回调 — 更新详情面板和位字段图 */
void SvdViewerWidget::onTreeSelectionChanged()
{
    QModelIndex current = m_treeView->currentIndex();
    if (!current.isValid()) {
        m_detailView->clear();
        m_bitFieldWidget->clear();
        return;
    }

    /* 获取选中节点的完整路径(外设.寄存器.字段) */
    QStringList pathParts;
    QModelIndex idx = current;
    while (idx.isValid()) {
        QString name = m_treeModel->data(
            m_treeModel->index(idx.row(),
                               SvdRegisterTreeModel::ColName,
                               idx.parent()),
            Qt::DisplayRole).toString();
        pathParts.prepend(name);
        idx = idx.parent();
    }

    QString peripheralName;
    QString registerName;

    if (pathParts.size() >= 2) {
        peripheralName = pathParts.value(1);
    }
    if (pathParts.size() >= 3) {
        registerName = pathParts.value(2);
    }

    updateDetailPanel(peripheralName, registerName);

    if (!registerName.isEmpty()) {
        emit registerSelected(peripheralName, registerName);
    }
}

/** @brief 树节点展开回调 — 递增展开统计 @param index 展开的节点索引 */
void SvdViewerWidget::onTreeExpanded(const QModelIndex& index)
{
    Q_UNUSED(index)
    ++m_totalExpandCount;
}

/** @brief 树节点折叠回调 — 递增折叠统计 @param index 折叠的节点索引 */
void SvdViewerWidget::onTreeCollapsed(const QModelIndex& index)
{
    Q_UNUSED(index)
    ++m_totalCollapseCount;
}

// ============================================================================
// 私有辅助
// ============================================================================

/** @brief 更新详情面板(显示选中项信息) @param peripheralName 外设名 @param registerName 寄存器名 */
void SvdViewerWidget::updateDetailPanel(const QString& peripheralName,
                                         const QString& registerName)
{
    if (peripheralName.isEmpty() && registerName.isEmpty()) {
        m_detailView->setPlainText(
            tr("选择外设或寄存器查看详情"));
        return;
    }

    QModelIndex current = m_treeView->currentIndex();
    if (!current.isValid()) { return; }

    /* 构建详情HTML */
    QString html;
    html += tr("<h3>%1</h3>").arg(
        m_treeModel->data(m_treeModel->index(
            current.row(), SvdRegisterTreeModel::ColName,
            current.parent()),
            Qt::DisplayRole).toString());

    /* 地址 */
    QString addr = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColAddress,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!addr.isEmpty()) {
        html += tr("<b>地址:</b> %1<br>").arg(addr);
    }

    /* 偏移 */
    QString offset = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColOffset,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!offset.isEmpty()) {
        html += tr("<b>偏移:</b> %1<br>").arg(offset);
    }

    /* 位宽 */
    QString size = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColSize,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!size.isEmpty()) {
        html += tr("<b>位宽:</b> %1<br>").arg(size);
    }

    /* 访问权限 */
    QString access = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColAccess,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!access.isEmpty()) {
        html += tr("<b>访问:</b> %1<br>").arg(access);
    }

    /* 复位值 */
    QString reset = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColResetValue,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!reset.isEmpty()) {
        html += tr("<b>复位值:</b> %1<br>").arg(reset);
    }

    /* 描述 */
    QString desc = m_treeModel->data(m_treeModel->index(
        current.row(), SvdRegisterTreeModel::ColDescription,
        current.parent()),
        Qt::DisplayRole).toString();
    if (!desc.isEmpty()) {
        html += tr("<b>描述:</b> %1").arg(desc);
    }

    m_detailView->setHtml(html);

    /* TODO: 当SvdParser就绪后，传入实际SvdRegister到bitFieldWidget */
    m_bitFieldWidget->clear();
}

/** @brief 重建外设过滤器下拉列表(从模型读取外设名称) */
void SvdViewerWidget::rebuildFilterCombo()
{
    m_filterCombo->clear();
    m_filterCombo->addItem(tr("全部外设"));

    /* TODO: 当模型加载后，遍历顶层外设节点填充下拉列表 */
}

// 统计getter和resetStatistics见 SvdViewerWidgetStats.cpp

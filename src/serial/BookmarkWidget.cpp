/**
 * @file BookmarkWidget.cpp
 * @brief 书签管理面板实现 - 嵌入导航树的书签浏览和管理界面
 */

#include "BookmarkWidget.h"

#include <QMessageBox>

/**
 * @brief 构造书签管理面板
 *
 * 布局结构:
 *   QVBoxLayout（主布局）
 *     |-- QLabel m_titleLabel        "书签列表" 标题
 *     |-- QHBoxLayout（工具栏按钮行）
 *     |     |-- QPushButton m_addBtn     添加书签
 *     |     |-- QPushButton m_removeBtn  删除选中
 *     |     |-- stretch                  弹性空白
 *     |     |-- QPushButton m_clearBtn   清空全部
 *     |-- QListWidget m_listWidget   书签列表
 *
 * 所有控件设置 objectName 以便 QSS 选择器匹配样式。
 */
BookmarkWidget::BookmarkWidget(QWidget* parent)
    : QWidget(parent)
{
    // ---- 主布局: 垂直排列，与 SerialConfigPanel 等面板风格一致 ----
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // ---- 标题标签 ----
    m_titleLabel = new QLabel(tr("书签列表"), this);
    m_titleLabel->setObjectName("bookmarkTitleLabel");
    mainLayout->addWidget(m_titleLabel);

    // ---- 工具栏按钮行 ----
    auto* toolbarLayout = new QHBoxLayout;
    toolbarLayout->setSpacing(6);

    // 添加书签按钮 - accent 色高亮，主要操作
    m_addBtn = new QPushButton(tr("添加"), this);
    m_addBtn->setObjectName("bookmarkAddBtn");
    m_addBtn->setMinimumHeight(28);
    m_addBtn->setToolTip(tr("在当前时间点添加一个新书签"));
    connect(m_addBtn, &QPushButton::clicked, this, &BookmarkWidget::onAddClicked);
    toolbarLayout->addWidget(m_addBtn);

    // 删除选中书签按钮 - 默认禁用，选中项后启用
    m_removeBtn = new QPushButton(tr("删除"), this);
    m_removeBtn->setObjectName("bookmarkRemoveBtn");
    m_removeBtn->setMinimumHeight(28);
    m_removeBtn->setEnabled(false);
    m_removeBtn->setToolTip(tr("删除选中的书签"));
    connect(m_removeBtn, &QPushButton::clicked, this, &BookmarkWidget::onRemoveClicked);
    toolbarLayout->addWidget(m_removeBtn);

    // 弹性空白，将清空按钮推到右侧
    toolbarLayout->addStretch();

    // 清空所有书签按钮 - 危险操作，红色警告风格
    m_clearBtn = new QPushButton(tr("清空"), this);
    m_clearBtn->setObjectName("bookmarkClearBtn");
    m_clearBtn->setMinimumHeight(28);
    m_clearBtn->setEnabled(false);
    m_clearBtn->setToolTip(tr("清空所有书签"));
    connect(m_clearBtn, &QPushButton::clicked, this, &BookmarkWidget::onClearClicked);
    toolbarLayout->addWidget(m_clearBtn);

    mainLayout->addLayout(toolbarLayout);

    // ---- 书签列表 ----
    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("bookmarkList");
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setToolTip(tr("双击书签跳转到对应时间位置"));
    // 列表项选中状态变化时更新删除按钮启用状态
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &BookmarkWidget::updateButtonStates);
    // 双击列表项发射跳转信号
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &BookmarkWidget::onItemDoubleClicked);
    mainLayout->addWidget(m_listWidget);

    // 设置自身 objectName 便于 QSS 选择
    setObjectName("bookmarkWidget");
}

/**
 * @brief 刷新书签列表显示
 *
 * 用最新的书签集合完全替换当前列表内容。
 * 每个列表项的文本格式为: "HH:mm:ss.zzz  标签文本"
 * 通过 setData(Qt::UserRole) 存储书签在集合中的索引，
 * 以便删除和双击时获取正确的索引。
 *
 * @param bookmarks 最新的书签集合（按添加顺序）
 */
void BookmarkWidget::refreshBookmarks(const QVector<DataBookmark>& bookmarks)
{
    // 保存当前选中项索引（如果有的话）
    int selectedRow = -1;
    if (auto* current = m_listWidget->currentItem()) {
        selectedRow = current->data(Qt::UserRole).toInt();
    }

    // 清除旧内容
    m_listWidget->clear();

    // 逐条添加新书签项
    for (int i = 0; i < bookmarks.size(); ++i) {
        const auto& bm = bookmarks[i];
        const QString text = formatTimestamp(bm.timestamp)
                           + QStringLiteral("  ")
                           + bm.label;
        auto* item = new QListWidgetItem(text, m_listWidget);
        // UserRole 存储原始索引，供删除/双击时使用
        item->setData(Qt::UserRole, i);
        // 工具提示: 显示完整时间戳和标签
        const QString fullTime = QDateTime::fromMSecsSinceEpoch(bm.timestamp)
                                     .toString("yyyy-MM-dd HH:mm:ss.zzz");
        item->setToolTip(fullTime + QStringLiteral("\n") + bm.label);
    }

    // 尝试恢复选中项（如果索引仍有效）
    if (selectedRow >= 0 && selectedRow < m_listWidget->count()) {
        m_listWidget->setCurrentRow(selectedRow);
    }

    // 更新按钮启用状态
    updateButtonStates();
}

/**
 * @brief 处理添加书签按钮点击
 *
 * 弹出简单的输入对话框，让用户输入书签标签。
 * 对话框包含:
 *   - QLineEdit 用于输入标签文本
 *   - QDialogButtonBox 提供"确定"和"取消"按钮
 *
 * 用户确认后发射 addBookmarkRequested 信号，携带输入的标签文本。
 * 如果用户输入为空，使用"书签"作为默认标签。
 */
void BookmarkWidget::onAddClicked()
{
    // ---- 标签输入对话框 ----
    QDialog dlg(window());
    dlg.setWindowTitle(tr("添加书签"));
    dlg.setObjectName("bookmarkAddDlg");
    dlg.setMinimumWidth(320);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(8);

    // 提示标签
    auto* hintLabel = new QLabel(tr("请输入书签标签:"), &dlg);
    hintLabel->setObjectName("bookmarkDlgHint");
    layout->addWidget(hintLabel);

    // 标签输入框
    auto* labelInput = new QLineEdit(&dlg);
    labelInput->setObjectName("bookmarkLabelInput");
    labelInput->setPlaceholderText(tr("例如: 异常发生、复位完成"));
    labelInput->setMinimumHeight(28);
    layout->addWidget(labelInput);

    // 确定/取消按钮
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->setObjectName("bookmarkDlgButtons");
    layout->addWidget(buttons);

    // 连接按钮信号
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // 输入框获得焦点，方便用户直接输入
    labelInput->setFocus();

    // 显示对话框
    if (dlg.exec() == QDialog::Accepted) {
        QString label = labelInput->text().trimmed();
        // 输入为空时使用默认标签
        if (label.isEmpty()) {
            label = tr("书签");
        }
        emit addBookmarkRequested(label);
    }
}

/**
 * @brief 处理删除选中书签按钮点击
 *
 * 获取当前选中的列表项，从中提取存储在 Qt::UserRole 中的索引，
 * 发射 removeBookmarkRequested 信号。
 * 如果没有选中项，忽略此次操作。
 */
void BookmarkWidget::onRemoveClicked()
{
    auto* current = m_listWidget->currentItem();
    if (!current) {
        return;
    }

    const int index = current->data(Qt::UserRole).toInt();
    emit removeBookmarkRequested(index);
}

/**
 * @brief 处理清空所有书签按钮点击
 *
 * 弹出确认对话框，防止误操作。用户确认后发射 clearBookmarksRequested 信号。
 * 如果当前列表为空，忽略此次操作。
 */
void BookmarkWidget::onClearClicked()
{
    if (m_listWidget->count() == 0) {
        return;
    }

    // 确认对话框，防止误触清空
    const int ret = QMessageBox::question(
        this,
        tr("清空书签"),
        tr("确定要清空所有书签吗？此操作不可撤销。"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        emit clearBookmarksRequested();
    }
}

/**
 * @brief 处理列表项双击
 *
 * 从双击的列表项中提取存储在 Qt::UserRole 中的索引，
 * 发射 bookmarkDoubleClicked 信号。
 * 未来此信号将连接到时间轴跳转逻辑，当前仅记录信号发射。
 *
 * @param item 双击的列表项
 */
void BookmarkWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) {
        return;
    }

    const int index = item->data(Qt::UserRole).toInt();
    emit bookmarkDoubleClicked(index);
}

/**
 * @brief 将书签时间戳格式化为可读字符串
 *
 * 将 Unix 纪元毫秒时间戳转换为 "HH:mm:ss.zzz" 格式，
 * 精确到毫秒，便于用户在时间轴上定位。
 *
 * @param timestampMs 毫秒级 Unix 时间戳（ms since epoch）
 * @return 格式化后的时间字符串，如 "14:30:25.123"
 */
QString BookmarkWidget::formatTimestamp(qint64 timestampMs)
{
    return QDateTime::fromMSecsSinceEpoch(timestampMs)
               .toString("HH:mm:ss.zzz");
}

/**
 * @brief 更新删除按钮和清空按钮的启用状态
 *
 * 规则:
 *   - 删除按钮: 列表中有选中项时启用，否则禁用
 *   - 清空按钮: 列表中至少有一个书签时启用，否则禁用
 *
 * 此方法在以下时机被调用:
 *   - refreshBookmarks() 刷新列表后
 *   - QListWidget::itemSelectionChanged 信号触发时
 */
void BookmarkWidget::updateButtonStates()
{
    // 删除按钮: 需要有选中项
    const bool hasSelection = (m_listWidget->currentItem() != nullptr);
    m_removeBtn->setEnabled(hasSelection);

    // 清空按钮: 需要列表非空
    const bool hasItems = (m_listWidget->count() > 0);
    m_clearBtn->setEnabled(hasItems);
}

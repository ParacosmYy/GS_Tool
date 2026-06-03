/**
 * @file BookmarkWidget.cpp
 * @brief 书签管理面板实现 - 嵌入导航树的书签浏览和管理界面
 */

#include "serial/data/BookmarkWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "core/widgets/EdDialog.h"

BookmarkWidget::BookmarkWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    m_titleLabel = new QLabel(tr("书签列表"), this);
    m_titleLabel->setObjectName("bookmarkTitleLabel");
    mainLayout->addWidget(m_titleLabel);

    auto* toolbarLayout = new QHBoxLayout;
    toolbarLayout->setSpacing(6);

    m_addBtn = new AnimatedButton(tr("添加"), this);
    m_addBtn->setObjectName("bookmarkAddBtn");
    m_addBtn->setMinimumHeight(28);
    m_addBtn->setToolTip(tr("在当前时间点添加一个新书签"));
    connect(m_addBtn, &QPushButton::clicked, this, &BookmarkWidget::onAddClicked);
    toolbarLayout->addWidget(m_addBtn);

    m_removeBtn = new AnimatedButton(tr("删除"), this);
    m_removeBtn->setObjectName("bookmarkRemoveBtn");
    m_removeBtn->setMinimumHeight(28);
    m_removeBtn->setEnabled(false);
    m_removeBtn->setToolTip(tr("删除选中的书签"));
    connect(m_removeBtn, &QPushButton::clicked, this, &BookmarkWidget::onRemoveClicked);
    toolbarLayout->addWidget(m_removeBtn);

    toolbarLayout->addStretch();

    m_clearBtn = new AnimatedButton(tr("清空"), this);
    m_clearBtn->setObjectName("bookmarkClearBtn");
    m_clearBtn->setMinimumHeight(28);
    m_clearBtn->setEnabled(false);
    m_clearBtn->setToolTip(tr("清空所有书签"));
    connect(m_clearBtn, &QPushButton::clicked, this, &BookmarkWidget::onClearClicked);
    toolbarLayout->addWidget(m_clearBtn);

    mainLayout->addLayout(toolbarLayout);

    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("bookmarkList");
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setToolTip(tr("双击书签跳转到对应时间位置"));
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &BookmarkWidget::updateButtonStates);
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &BookmarkWidget::onItemDoubleClicked);
    mainLayout->addWidget(m_listWidget);

    setObjectName("bookmarkWidget");
}

/** @brief 刷新书签列表显示 — 累计刷新统计 */
void BookmarkWidget::refreshBookmarks(const QVector<DataBookmark>& bookmarks)
{
    ++m_totalRefreshes;
    int selectedRow = -1;
    if (auto* current = m_listWidget->currentItem()) {
        selectedRow = current->data(Qt::UserRole).toInt();
    }

    m_listWidget->clear();

    for (int i = 0; i < bookmarks.size(); ++i) {
        const auto& bm = bookmarks[i];
        const QString text = formatTimestamp(bm.timestamp)
                           + QStringLiteral("  ")
                           + bm.label;
        auto* item = new QListWidgetItem(text, m_listWidget);
        item->setData(Qt::UserRole, i);
        const QString fullTime = QDateTime::fromMSecsSinceEpoch(bm.timestamp)
                                     .toString("yyyy-MM-dd HH:mm:ss.zzz");
        item->setToolTip(fullTime + QStringLiteral("\n") + bm.label);
    }

    if (selectedRow >= 0 && selectedRow < m_listWidget->count()) {
        m_listWidget->setCurrentRow(selectedRow);
    }
    updateButtonStates();
}

/** @brief 处理添加书签按钮点击 — 累计添加统计 */
void BookmarkWidget::onAddClicked()
{
    QDialog dlg(window());
    dlg.setWindowTitle(tr("添加书签"));
    dlg.setObjectName("bookmarkAddDlg");
    dlg.setMinimumWidth(320);

    auto* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(8);

    auto* hintLabel = new QLabel(tr("请输入书签标签:"), &dlg);
    hintLabel->setObjectName("bookmarkDlgHint");
    layout->addWidget(hintLabel);

    auto* labelInput = new QLineEdit(&dlg);
    labelInput->setObjectName("bookmarkLabelInput");
    labelInput->setPlaceholderText(tr("例如: 异常发生、复位完成"));
    labelInput->setMinimumHeight(28);
    layout->addWidget(labelInput);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->setObjectName("bookmarkDlgButtons");
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    labelInput->setFocus();

    if (dlg.exec() == QDialog::Accepted) {
        QString label = labelInput->text().trimmed();
        if (label.isEmpty()) {
            label = tr("书签");
        }
        ++m_totalBookmarksAdded;
        emit addBookmarkRequested(label);
    }
}

/** @brief 处理删除选中书签 — 累计删除统计 */
void BookmarkWidget::onRemoveClicked()
{
    auto* current = m_listWidget->currentItem();
    if (!current) return;

    const int index = current->data(Qt::UserRole).toInt();
    ++m_totalBookmarksRemoved;
    emit removeBookmarkRequested(index);
}

/** @brief 处理清空所有书签按钮点击 */
void BookmarkWidget::onClearClicked()
{
    if (m_listWidget->count() == 0) return;

    if (EdDialog::confirm(this, tr("清空书签"), tr("确定要清空所有书签吗？此操作不可撤销。"))) {
        emit clearBookmarksRequested();
    }
}

/** @brief 处理列表项双击 */
void BookmarkWidget::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!item) return;
    const int index = item->data(Qt::UserRole).toInt();
    emit bookmarkDoubleClicked(index);
}

/** @brief 格式化时间戳为 "HH:mm:ss.zzz" */
QString BookmarkWidget::formatTimestamp(qint64 timestampMs)
{
    return QDateTime::fromMSecsSinceEpoch(timestampMs).toString("HH:mm:ss.zzz");
}

/** @brief 更新删除/清空按钮启用状态 */
void BookmarkWidget::updateButtonStates()
{
    m_removeBtn->setEnabled(m_listWidget->currentItem() != nullptr);
    m_clearBtn->setEnabled(m_listWidget->count() > 0);
}

quint64 BookmarkWidget::totalBookmarksAdded() const { return m_totalBookmarksAdded; }
quint64 BookmarkWidget::totalBookmarksRemoved() const { return m_totalBookmarksRemoved; }
quint64 BookmarkWidget::totalRefreshes() const { return m_totalRefreshes; }

void BookmarkWidget::resetBookmarkStats()
{
    m_totalBookmarksAdded = 0;
    m_totalBookmarksRemoved = 0;
    m_totalRefreshes = 0;
}

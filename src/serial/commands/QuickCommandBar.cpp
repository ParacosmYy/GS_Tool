/**
 * @file QuickCommandBar.cpp
 * @brief 快捷指令栏实现 - UI构建、按钮重建与编辑对话框
 *
 * 指令列表管理、持久化存储与统计查询已拆分至 QuickCommandBarActions.cpp。
 */

#include "serial/commands/QuickCommandBar.h"
#include "core/widgets/AnimatedButton.h"
#include "utils/crypto/HexConverter.h"

#include <QSettings>
#include <QVBoxLayout>

/** @brief 构造快捷指令栏(加载保存的指令+构建按钮行) @param parent 父控件 */
QuickCommandBar::QuickCommandBar(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(6);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(6);
    mainLayout->addLayout(m_buttonLayout);

    mainLayout->addStretch();

    // ---- 编辑按钮 ----
    // 使用 setMinimumHeight(32) 替代 setFixedSize，允许水平自适应内容
    // 与 SerialConfigPanel 连接按钮(36px)视觉协调: 次要操作按钮略矮
    m_editBtn = new AnimatedButton(tr("编辑"));
    m_editBtn->setObjectName("quickCmdEditBtn");
    m_editBtn->setMinimumHeight(32);
    connect(m_editBtn, &QPushButton::clicked, this, &QuickCommandBar::onEditRequested);
    mainLayout->addWidget(m_editBtn);

    // ---- 添加按钮 ----
    // 使用 setMinimumHeight(32) 替代 setFixedSize(32,32)，保持与其他按钮等高
    m_addBtn = new AnimatedButton(tr("+"));
    m_addBtn->setObjectName("quickCmdAddBtn");
    m_addBtn->setMinimumHeight(32);
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        QuickCommand cmd{tr("指令"), "", false};
        addCommand(cmd);
    });
    mainLayout->addWidget(m_addBtn);

    setObjectName("quickCommandBar");
}

// setCommands/addCommand/clearCommands 已移至 QuickCommandBarActions.cpp

/**
 * @brief 根据当前 m_commands 列表重建所有快捷指令按钮
 *
 * 先清除 m_buttonLayout 中的旧按钮（deleteLater安全销毁），
 * 再为每条指令创建新按钮，设置 objectName 和 dynamic property
 * 以便 QSS 选择器匹配样式。
 *
 * 每个按钮通过 connect 绑定点击事件:
 *   - HEX 模式: 使用 HexConverter::fromHexString 转换后发射
 *   - 文本模式: 使用 toUtf8() 转换后发射
 *   - 数据为空时不发射信号
 *
 * 点击时更新统计计数器:
 *   - m_totalCommandsSent: 每次有效发送 +1
 *   - m_totalQuickSends: 累加发送字节数
 *   - m_maxCommandLength: 跟踪最大单条指令长度
 */
void QuickCommandBar::rebuildButtons()
{
    // 清除旧按钮（安全销毁旧 QWidget）
    QLayoutItem* item;
    while ((item = m_buttonLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // 为每条指令创建新按钮
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        auto* btn = new AnimatedButton(cmd.name);
        btn->setObjectName(QString("quickCmdBtn_%1").arg(i));
        // dynamic property 供 QSS 选择器 [quickCmdBtn="true"] 匹配
        btn->setProperty("quickCmdBtn", true);
        // 最小80x32，最大宽度160px，与工具栏按钮高度协调
        btn->setMinimumSize(80, 32);
        btn->setMaximumWidth(160);
        // 工具提示: 数据非空时显示原始数据，否则显示名称
        btn->setToolTip(cmd.data.isEmpty() ? cmd.name : cmd.data);

        // 点击按钮时发送数据并更新统计计数器
        connect(btn, &QPushButton::clicked, this, [this, cmd]() {
            QByteArray data;
            if (cmd.isHex) {
                // HEX模式: 将十六进制字符串转为原始字节
                data = HexConverter::fromHexString(cmd.data);
                if (data.isEmpty() && !cmd.data.isEmpty()) {
                    // HEX格式无效(如"ZZ")，通知用户而非静默忽略
                    emit commandError(tr("HEX格式无效: 指令'%1'的数据'%2'不是合法的十六进制")
                                      .arg(cmd.name, cmd.data));
                    return;
                }
            } else {
                // 文本模式: 将字符串转为UTF-8字节
                data = cmd.data.toUtf8();
            }
            if (!data.isEmpty()) {
                // 更新统计: 发送次数 +1，累计字节数，最大指令长度
                ++m_totalCommandsSent;
                m_totalQuickSends += static_cast<quint64>(data.size());
                m_maxCommandLength = qMax(m_maxCommandLength,
                                          static_cast<quint64>(data.size()));
                emit commandTriggered(data);
            }
        });

        m_buttonLayout->addWidget(btn);
    }
}

/**
 * @brief 打开指令编辑对话框，支持增删改指令
 *
 * 使用 QTableWidget 实现简单的表格编辑: 名称 | 数据 | HEX
 * 对话框内按钮设置 objectName 以便 QSS 定制样式。
 */
void QuickCommandBar::onEditRequested()
{
    // ---- 指令编辑对话框 ----
    QDialog dlg(window());
    QTableWidget* table = nullptr;
    QDialogButtonBox* buttons = nullptr;
    createEditDialog(dlg, table, buttons);
    populateDialogFields(table);

    // 用户确认后，从表格读回数据到指令列表
    if (dlg.exec() == QDialog::Accepted) {
        QList<QuickCommand> newCmds;
        for (int i = 0; i < table->rowCount(); ++i) {
            QuickCommand cmd;
            auto* nameItem = table->item(i, 0);
            auto* dataItem = table->item(i, 1);
            auto* hexItem  = table->item(i, 2);
            cmd.name  = nameItem ? nameItem->text() : QString();
            cmd.data  = dataItem ? dataItem->text() : QString();
            cmd.isHex = hexItem ? (hexItem->checkState() == Qt::Checked) : false;
            // 跳过完全空的行
            if (!cmd.name.isEmpty() || !cmd.data.isEmpty()) {
                newCmds.append(cmd);
            }
        }
        setCommands(newCmds);
        // 编辑确认后持久化到QSettings，防止重启后丢失
        saveCommands();
    }

    // 同时发射editRequested信号，允许外部监听者做额外处理
    emit editRequested();
}

/** @brief 创建编辑对话框的UI控件(表格、按钮行、信号连接) */
void QuickCommandBar::createEditDialog(QDialog& dlg, QTableWidget*& outTable, QDialogButtonBox*& outButtons)
{
    auto* layout = new QVBoxLayout(&dlg);

    // 表格: 3列 — 名称、数据、HEX开关
    outTable = new QTableWidget(m_commands.size(), 3, &dlg);
    outTable->setObjectName("quickCmdEditTable");
    outTable->setHorizontalHeaderLabels({tr("名称"), tr("数据"), tr("HEX")});
    outTable->horizontalHeader()->setStretchLastSection(false);
    outTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    outTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    outTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    outTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(outTable);

    // ---- 按钮行: 添加行 / 删除行 / 确定 / 取消 ----
    auto* btnLayout = new QHBoxLayout;
    auto* addRowBtn = new QPushButton(tr("添加行"), &dlg);
    addRowBtn->setObjectName("quickCmdAddRowBtn");
    auto* delRowBtn = new QPushButton(tr("删除行"), &dlg);
    delRowBtn->setObjectName("quickCmdDelRowBtn");
    outButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    outButtons->setObjectName("quickCmdDlgButtons");  // QSS 选择器需要

    btnLayout->addWidget(addRowBtn);
    btnLayout->addWidget(delRowBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(outButtons);
    layout->addLayout(btnLayout);

    // 添加行按钮: 在表格末尾追加空行
    connect(addRowBtn, &QPushButton::clicked, this, [outTable]() {
        int row = outTable->rowCount();
        outTable->insertRow(row);
        outTable->setItem(row, 0, new QTableWidgetItem(tr("指令")));
        outTable->setItem(row, 1, new QTableWidgetItem{});
        auto* check = new QTableWidgetItem;
        check->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        check->setCheckState(Qt::Unchecked);
        outTable->setItem(row, 2, check);
    });

    // 删除行按钮: 删除选中行
    connect(delRowBtn, &QPushButton::clicked, this, [outTable]() {
        auto selected = outTable->selectionModel()->selectedRows();
        // 从后往前删，避免索引偏移
        for (int i = selected.size() - 1; i >= 0; --i) {
            outTable->removeRow(selected[i].row());
        }
    });

    // 确定/取消
    connect(outButtons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(outButtons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
}

/** @brief 将当前指令列表填充到编辑对话框表格中 */
void QuickCommandBar::populateDialogFields(QTableWidget* table)
{
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        table->setItem(i, 0, new QTableWidgetItem(cmd.name));
        table->setItem(i, 1, new QTableWidgetItem(cmd.data));
        auto* hexCheck = new QTableWidgetItem;
        hexCheck->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        hexCheck->setCheckState(cmd.isHex ? Qt::Checked : Qt::Unchecked);
        table->setItem(i, 2, hexCheck);
    }
}

// saveCommands/loadCommands 已移至 QuickCommandBarActions.cpp
// totalCommandsSent/totalQuickSends/maxCommandLength/resetStatistics 已移至 QuickCommandBarActions.cpp

/**
 * @file QuickCommandBarDialog.cpp
 * @brief 快捷指令栏 - 编辑对话框实现
 *
 * 从 QuickCommandBar.cpp 拆分而来，包含按钮重建、编辑对话框创建、
 * 字段填充和编辑请求处理方法。
 */

#include "serial/commands/QuickCommandBar.h"
#include "core/widgets/AnimatedButton.h"
#include "utils/crypto/HexConverter.h"

#include <QSettings>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialogButtonBox>

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
        btn->setProperty("quickCmdBtn", true);
        btn->setMinimumSize(80, 32);
        btn->setMaximumWidth(160);
        btn->setToolTip(cmd.data.isEmpty() ? cmd.name : cmd.data);

        // 点击按钮时发送数据并更新统计计数器
        connect(btn, &QPushButton::clicked, this, [this, cmd]() {
            QByteArray data;
            if (cmd.isHex) {
                data = HexConverter::fromHexString(cmd.data);
                if (data.isEmpty() && !cmd.data.isEmpty()) {
                    emit commandError(tr("HEX格式无效: 指令'%1'的数据'%2'不是合法的十六进制")
                                      .arg(cmd.name, cmd.data));
                    return;
                }
                ++m_totalHexCommands;
            } else {
                data = cmd.data.toUtf8();
            }
            if (!data.isEmpty()) {
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

void QuickCommandBar::onEditRequested()
{
    ++m_totalEditDialogOpens;
    QDialog dlg(window());
    QTableWidget* table = nullptr;
    QDialogButtonBox* buttons = nullptr;
    createEditDialog(dlg, table, buttons);
    populateDialogFields(table);

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
            if (!cmd.name.isEmpty() || !cmd.data.isEmpty()) {
                newCmds.append(cmd);
            }
        }
        /* 统计: 多条指令批量保存视为一次宏运行 */
        if (newCmds.size() > 1) ++m_totalMacrosRun;
        setCommands(newCmds);
        saveCommands();
    }

    emit editRequested();
}

/** @brief 创建编辑对话框的UI控件(表格、按钮行、信号连接) */
void QuickCommandBar::createEditDialog(QDialog& dlg, QTableWidget*& outTable, QDialogButtonBox*& outButtons)
{
    auto* layout = new QVBoxLayout(&dlg);

    outTable = new QTableWidget(m_commands.size(), 3, &dlg);
    outTable->setObjectName("quickCmdEditTable");
    outTable->setHorizontalHeaderLabels({tr("名称"), tr("数据"), tr("HEX")});
    outTable->horizontalHeader()->setStretchLastSection(false);
    outTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    outTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    outTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    outTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(outTable);

    auto* btnLayout = new QHBoxLayout;
    auto* addRowBtn = new QPushButton(tr("添加行"), &dlg);
    addRowBtn->setObjectName("quickCmdAddRowBtn");
    auto* delRowBtn = new QPushButton(tr("删除行"), &dlg);
    delRowBtn->setObjectName("quickCmdDelRowBtn");
    outButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    outButtons->setObjectName("quickCmdDlgButtons");

    btnLayout->addWidget(addRowBtn);
    btnLayout->addWidget(delRowBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(outButtons);
    layout->addLayout(btnLayout);

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

    connect(delRowBtn, &QPushButton::clicked, this, [outTable]() {
        auto selected = outTable->selectionModel()->selectedRows();
        for (int i = selected.size() - 1; i >= 0; --i) {
            outTable->removeRow(selected[i].row());
        }
    });

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

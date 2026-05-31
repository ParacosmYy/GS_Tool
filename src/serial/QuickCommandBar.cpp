/**
 * @file QuickCommandBar.cpp
 * @brief 快捷指令栏实现 - 可配置的底部按钮行，点击即发送预设命令
 */

#include "QuickCommandBar.h"
#include "utils/HexConverter.h"

#include <QSettings>
#include <QVBoxLayout>

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
    m_editBtn = new QPushButton(tr("Edit"));
    m_editBtn->setObjectName("quickCmdEditBtn");
    m_editBtn->setMinimumHeight(32);
    connect(m_editBtn, &QPushButton::clicked, this, &QuickCommandBar::onEditRequested);
    mainLayout->addWidget(m_editBtn);

    // ---- 添加按钮 ----
    // 使用 setMinimumHeight(32) 替代 setFixedSize(32,32)，保持与其他按钮等高
    m_addBtn = new QPushButton(tr("+"));
    m_addBtn->setObjectName("quickCmdAddBtn");
    m_addBtn->setMinimumHeight(32);
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        QuickCommand cmd{tr("Command"), "", false};
        addCommand(cmd);
    });
    mainLayout->addWidget(m_addBtn);

    setObjectName("quickCommandBar");
}

/**
 * @brief 设置指令列表，替换当前全部指令并重建按钮
 * @param commands 新的指令列表
 */
void QuickCommandBar::setCommands(const QList<QuickCommand>& commands)
{
    m_commands = commands;
    rebuildButtons();
}

/** @brief 获取当前指令列表的副本 */
QList<QuickCommand> QuickCommandBar::commands() const
{
    return m_commands;
}

/**
 * @brief 添加一条指令到列表末尾并重建按钮
 * @param cmd 要添加的指令
 */
void QuickCommandBar::addCommand(const QuickCommand& cmd)
{
    m_commands.append(cmd);
    rebuildButtons();
}

/** @brief 清空所有指令并移除按钮 */
void QuickCommandBar::clearCommands()
{
    m_commands.clear();
    rebuildButtons();
}

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
        auto* btn = new QPushButton(cmd.name);
        btn->setObjectName(QString("quickCmdBtn_%1").arg(i));
        // dynamic property 供 QSS 选择器 [quickCmdBtn="true"] 匹配
        btn->setProperty("quickCmdBtn", true);
        // 最小80x32，最大宽度160px，与工具栏按钮高度协调
        btn->setMinimumSize(80, 32);
        btn->setMaximumWidth(160);
        // 工具提示: 数据非空时显示原始数据，否则显示名称
        btn->setToolTip(cmd.data.isEmpty() ? cmd.name : cmd.data);

        // 点击按钮时发送数据（lambda 捕获 cmd 副本）
        connect(btn, &QPushButton::clicked, this, [this, cmd]() {
            QByteArray data;
            if (cmd.isHex) {
                // HEX 模式: 将十六进制字符串转为原始字节
                data = HexConverter::fromHexString(cmd.data);
            } else {
                // 文本模式: 将字符串转为 UTF-8 字节
                data = cmd.data.toUtf8();
            }
            if (!data.isEmpty()) {
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
    dlg.setWindowTitle(tr("编辑快捷指令"));
    dlg.setMinimumSize(480, 320);
    dlg.setObjectName("quickCmdEditDlg");

    auto* layout = new QVBoxLayout(&dlg);

    // 表格: 3列 — 名称、数据、HEX开关
    auto* table = new QTableWidget(m_commands.size(), 3, &dlg);
    table->setObjectName("quickCmdEditTable");
    table->setHorizontalHeaderLabels({tr("名称"), tr("数据"), tr("HEX")});
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 填充现有指令到表格
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        table->setItem(i, 0, new QTableWidgetItem(cmd.name));
        table->setItem(i, 1, new QTableWidgetItem(cmd.data));
        auto* hexCheck = new QTableWidgetItem;
        hexCheck->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        hexCheck->setCheckState(cmd.isHex ? Qt::Checked : Qt::Unchecked);
        table->setItem(i, 2, hexCheck);
    }

    layout->addWidget(table);

    // ---- 按钮行: 添加行 / 删除行 / 确定 / 取消 ----
    auto* btnLayout = new QHBoxLayout;
    auto* addRowBtn = new QPushButton(tr("添加行"), &dlg);
    addRowBtn->setObjectName("quickCmdAddRowBtn");
    auto* delRowBtn = new QPushButton(tr("删除行"), &dlg);
    delRowBtn->setObjectName("quickCmdDelRowBtn");
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->setObjectName("quickCmdDlgButtons");  // QSS 选择器需要

    btnLayout->addWidget(addRowBtn);
    btnLayout->addWidget(delRowBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(buttons);
    layout->addLayout(btnLayout);

    // 添加行按钮: 在表格末尾追加空行
    connect(addRowBtn, &QPushButton::clicked, this, [table]() {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(tr("指令")));
        table->setItem(row, 1, new QTableWidgetItem{});
        auto* check = new QTableWidgetItem;
        check->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        check->setCheckState(Qt::Unchecked);
        table->setItem(row, 2, check);
    });

    // 删除行按钮: 删除选中行
    connect(delRowBtn, &QPushButton::clicked, this, [table]() {
        auto selected = table->selectionModel()->selectedRows();
        // 从后往前删，避免索引偏移
        for (int i = selected.size() - 1; i >= 0; --i) {
            table->removeRow(selected[i].row());
        }
    });

    // 确定/取消
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

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
    }

    // 同时发射editRequested信号，允许外部监听者做额外处理
    emit editRequested();
}

/**
 * @brief 将当前指令列表保存到 QSettings
 *
 * 使用 "QuickCommands" 组存储，格式如下:
 *   count  = 指令总数
 *   name_0 = 第一条指令的名称
 *   data_0 = 第一条指令的数据
 *   hex_0  = "1" 或 "0"（HEX 模式开关）
 *   name_1 = 第二条指令的名称
 *   ...以此类推
 *
 * 保存前会清除该组中所有旧数据，避免残留。
 */
void QuickCommandBar::saveCommands()
{
    QSettings settings;
    settings.beginGroup("QuickCommands");

    // 清除旧数据，防止删除指令后残留
    settings.remove("");

    // 写入指令总数
    settings.setValue("count", m_commands.size());

    // 逐条写入指令
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        settings.setValue(QString("name_%1").arg(i), cmd.name);
        settings.setValue(QString("data_%1").arg(i), cmd.data);
        settings.setValue(QString("hex_%1").arg(i), cmd.isHex ? "1" : "0");
    }

    settings.endGroup();
    settings.sync();
}

/**
 * @brief 从 QSettings 加载指令列表
 *
 * 读取 "QuickCommands" 组中保存的指令数据。
 * 如果 "count" 键不存在（首次使用或从未保存），直接返回不做任何操作。
 * 加载成功后替换内存中的指令列表并重建按钮。
 */
void QuickCommandBar::loadCommands()
{
    QSettings settings;
    settings.beginGroup("QuickCommands");

    // 检查是否有保存的数据
    if (!settings.contains("count")) {
        settings.endGroup();
        return;
    }

    const int count = settings.value("count", 0).toInt();
    QList<QuickCommand> loadedCommands;
    loadedCommands.reserve(count);

    for (int i = 0; i < count; ++i) {
        QuickCommand cmd;
        cmd.name  = settings.value(QString("name_%1").arg(i)).toString();
        cmd.data  = settings.value(QString("data_%1").arg(i)).toString();
        cmd.isHex = settings.value(QString("hex_%1").arg(i)).toString() == "1";

        // 跳过完全无效的条目（名称和数据都为空）
        if (!cmd.name.isEmpty() || !cmd.data.isEmpty()) {
            loadedCommands.append(cmd);
        }
    }

    settings.endGroup();

    // 替换当前指令列表并重建按钮
    setCommands(loadedCommands);
}

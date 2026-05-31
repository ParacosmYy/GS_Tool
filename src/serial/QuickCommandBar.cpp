#include "QuickCommandBar.h"
#include "utils/HexConverter.h"

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

    // 编辑按钮
    m_editBtn = new QPushButton(tr("编辑"));
    m_editBtn->setFixedSize(56, 32);
    connect(m_editBtn, &QPushButton::clicked, this, &QuickCommandBar::editRequested);
    mainLayout->addWidget(m_editBtn);

    // 添加按钮
    m_addBtn = new QPushButton("+");
    m_addBtn->setFixedSize(32, 32);
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        QuickCommand cmd{tr("指令"), "", false};
        addCommand(cmd);
    });
    mainLayout->addWidget(m_addBtn);

    setObjectName("quickCommandBar");
}

void QuickCommandBar::setCommands(const QList<QuickCommand>& commands)
{
    m_commands = commands;
    rebuildButtons();
}

QList<QuickCommand> QuickCommandBar::commands() const
{
    return m_commands;
}

void QuickCommandBar::addCommand(const QuickCommand& cmd)
{
    m_commands.append(cmd);
    rebuildButtons();
}

void QuickCommandBar::clearCommands()
{
    m_commands.clear();
    rebuildButtons();
}

void QuickCommandBar::rebuildButtons()
{
    // 清除旧按钮
    QLayoutItem* item;
    while ((item = m_buttonLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // 创建新按钮
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        auto* btn = new QPushButton(cmd.name);
        btn->setMinimumSize(80, 32);
        btn->setMaximumWidth(160);
        btn->setToolTip(cmd.data.isEmpty() ? cmd.name : cmd.data);

        // 点击按钮时发送数据
        connect(btn, &QPushButton::clicked, this, [this, cmd]() {
            QByteArray data;
            if (cmd.isHex) {
                data = HexConverter::fromHexString(cmd.data);
            } else {
                data = cmd.data.toUtf8();
            }
            if (!data.isEmpty()) {
                emit commandTriggered(data);
            }
        });

        m_buttonLayout->addWidget(btn);
    }
}

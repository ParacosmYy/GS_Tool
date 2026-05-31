#include "QuickCommandBar.h"
#include "utils/HexConverter.h"

QuickCommandBar::QuickCommandBar(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    mainLayout->setSpacing(4);

    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->setSpacing(4);
    mainLayout->addLayout(m_buttonLayout);

    mainLayout->addStretch();

    // 编辑按钮
    m_editBtn = new QPushButton(tr("Edit"));
    m_editBtn->setFixedWidth(50);
    connect(m_editBtn, &QPushButton::clicked, this, &QuickCommandBar::editRequested);
    mainLayout->addWidget(m_editBtn);

    // 添加按钮
    m_addBtn = new QPushButton("+");
    m_addBtn->setFixedWidth(30);
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        QuickCommand cmd{tr("CMD"), "", false};
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
        btn->setMinimumWidth(60);
        btn->setMaximumWidth(120);

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

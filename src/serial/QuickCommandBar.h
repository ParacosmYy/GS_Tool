#ifndef QUICKCOMMANDBAR_H
#define QUICKCOMMANDBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMap>

// 快捷指令结构体
struct QuickCommand {
    QString name;           // 按钮上显示的名字
    QString data;           // 要发送的数据
    bool isHex = false;     // 是否为HEX格式
};

// 快捷指令栏 - 底部的可配置按钮栏，点击即发送预设命令
class QuickCommandBar : public QWidget {
    Q_OBJECT

public:
    explicit QuickCommandBar(QWidget* parent = nullptr);

    // 设置指令列表
    void setCommands(const QList<QuickCommand>& commands);

    // 获取当前指令列表
    QList<QuickCommand> commands() const;

    // 添加一条指令
    void addCommand(const QuickCommand& cmd);

    // 清空所有指令
    void clearCommands();

signals:
    // 用户点击某个快捷指令，发出要发送的数据
    void commandTriggered(const QByteArray& data);

    // 用户想编辑指令列表
    void editRequested();

private:
    void rebuildButtons();

    QList<QuickCommand> m_commands;
    QHBoxLayout* m_buttonLayout = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
};

#endif // QUICKCOMMANDBAR_H

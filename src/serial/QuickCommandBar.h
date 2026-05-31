#ifndef QUICKCOMMANDBAR_H
#define QUICKCOMMANDBAR_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMap>
#include <QDialog>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>

/**
 * @brief 快捷指令数据结构
 *
 * 每条快捷指令包含三个字段:
 *   - name: 按钮上显示的名称
 *   - data: 点击时要发送的原始数据（文本或HEX字符串）
 *   - isHex: 是否以 HEX 模式发送（true 时 data 内容为十六进制字符串）
 */
struct QuickCommand {
    QString name;           ///< 按钮上显示的名字
    QString data;           ///< 要发送的数据
    bool isHex = false;     ///< 是否为HEX格式
};

/**
 * @brief 快捷指令栏 - 底部的可配置按钮栏，点击即发送预设命令
 *
 * 提供可配置的快捷指令按钮行，支持:
 *   - 动态添加/删除/编辑指令
 *   - 文本模式或 HEX 模式发送
 *   - 通过 QSettings 持久化指令列表
 *
 * 协作关系:
 *   - SendController: 接收 commandTriggered 信号，将数据写入连接
 *   - SettingsManager: 通过 QSettings 保存/恢复指令列表
 */
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

    /**
     * @brief 将当前指令列表保存到 QSettings
     *
     * 保存到 "QuickCommands" 组，每条指令存储为三个字段:
     *   - "name_0", "name_1", ... : 指令名称
     *   - "data_0", "data_1", ... : 指令数据
     *   - "hex_0",  "hex_1",  ... : 是否HEX格式（"1"/"0"）
     *   - "count" : 指令总数
     */
    void saveCommands();

    /**
     * @brief 从 QSettings 加载指令列表
     *
     * 读取 "QuickCommands" 组中保存的指令数据，
     * 替换当前内存中的指令列表并重建按钮。
     * 如果没有保存的数据，列表保持不变。
     */
    void loadCommands();

signals:
    // 用户点击某个快捷指令，发出要发送的数据
    void commandTriggered(const QByteArray& data);

    // 用户想编辑指令列表（外部可监听，用于自定义编辑面板）
    void editRequested();

private slots:
    /** @brief 打开指令编辑对话框，支持增删改指令 */
    void onEditRequested();

private:
    void rebuildButtons();

    QList<QuickCommand> m_commands;
    QHBoxLayout* m_buttonLayout = nullptr;
    QPushButton* m_addBtn = nullptr;
    QPushButton* m_editBtn = nullptr;
};

#endif // QUICKCOMMANDBAR_H

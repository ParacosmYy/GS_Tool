/**
 * @file QuickCommandBar.h
 * @brief 快捷指令栏 - 底部可配置按钮栏，点击即发送预设命令
 */

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

    /** @brief 设置指令列表，替换当前全部指令并重建按钮 */
    void setCommands(const QList<QuickCommand>& commands);

    /** @brief 获取当前指令列表 */
    QList<QuickCommand> commands() const;

    /** @brief 添加一条指令到列表末尾并重建按钮 */
    void addCommand(const QuickCommand& cmd);

    /** @brief 清空所有指令并移除按钮 */
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
    /**
     * @brief 用户点击某个快捷指令时发射，携带要发送的数据
     * @param data 要发送的原始字节数据（已根据 isHex 完成转换）
     */
    void commandTriggered(const QByteArray& data);

    /** @brief 编辑对话框关闭后发射，通知外部做额外处理 */
    void editRequested();

private slots:
    /** @brief 打开指令编辑对话框，支持增删改指令 */
    void onEditRequested();

private:
    /**
     * @brief 根据当前 m_commands 列表重建所有快捷指令按钮
     *
     * 先清除 m_buttonLayout 中的旧按钮，再为每条指令创建新按钮。
     * 每个按钮通过 connect 绑定点击事件到 commandTriggered 信号。
     */
    void rebuildButtons();

    QList<QuickCommand> m_commands;     ///< 当前指令列表
    QHBoxLayout* m_buttonLayout = nullptr;  ///< 指令按钮的布局
    QPushButton* m_addBtn = nullptr;    ///< 快速添加按钮（objectName: quickCmdAddBtn）
    QPushButton* m_editBtn = nullptr;   ///< 打开编辑对话框按钮（objectName: quickCmdEditBtn）
};

#endif // QUICKCOMMANDBAR_H

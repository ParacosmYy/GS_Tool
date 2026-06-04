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
    /** @brief 构造快捷指令栏 @param parent 父控件 */
    explicit QuickCommandBar(QWidget* parent = nullptr);

    /** @brief 设置指令列表，替换当前全部指令并重建按钮 */
    void setCommands(const QList<QuickCommand>& commands);

    /** @brief 获取当前指令列表 */
    QList<QuickCommand> commands() const;

    /** @brief 添加一条指令到列表末尾并重建按钮 */
    void addCommand(const QuickCommand& cmd);

    /** @brief 清空所有指令并移除按钮 */
    void clearCommands();

    /** @brief 将当前指令列表保存到 QSettings */
    void saveCommands();

    /** @brief 从 QSettings 加载指令列表 */
    void loadCommands();

    /** @brief 获取快捷栏已发送的指令总次数 */
    quint64 totalCommandsSent() const;

    /** @brief 获取快捷发送累计发送的总字节数 */
    quint64 totalQuickSends() const;

    /** @brief 获取历史最大单条指令长度（字节数） */
    quint64 maxCommandLength() const;

    /** @brief 获取HEX模式指令触发总次数 @return HEX指令计数 */
    quint64 totalHexCommands() const { return m_totalHexCommands; }

    /** @brief 获取编辑对话框打开总次数 @return 编辑计数 */
    quint64 totalEditDialogOpens() const { return m_totalEditDialogOpens; }

    /** @brief 获取宏指令运行总次数 @return 宏运行计数 */
    quint64 totalMacrosRun() const { return m_totalMacrosRun; }

    /** @brief 重置所有统计计数器为零 */
    void resetStatistics();

signals:
    /** @brief 用户点击某个快捷指令时发射，携带要发送的数据 */
    void commandTriggered(const QByteArray& data);

    /** @brief 编辑对话框关闭后发射，通知外部做额外处理 */
    void editRequested();

    /** @brief HEX指令数据无效时发射，用于显示错误提示 @param msg 错误消息 */
    void commandError(const QString& msg);

private slots:
    /** @brief 打开指令编辑对话框，支持增删改指令 */
    void onEditRequested();

private:
    /** @brief 根据当前 m_commands 列表重建所有快捷指令按钮 */
    void rebuildButtons();

    /** @brief 创建编辑对话框UI(表格+按钮行+信号连接) */
    void createEditDialog(QDialog& dlg, QTableWidget*& table, QDialogButtonBox*& buttons);
    /** @brief 将当前指令填充到编辑对话框表格中 */
    void populateDialogFields(QTableWidget* table);

    QList<QuickCommand> m_commands;         ///< 当前指令列表
    QHBoxLayout* m_buttonLayout = nullptr;  ///< 指令按钮的布局
    QPushButton* m_addBtn = nullptr;        ///< 快速添加按钮（objectName: quickCmdAddBtn）
    QPushButton* m_editBtn = nullptr;       ///< 打开编辑对话框按钮（objectName: quickCmdEditBtn）

    quint64 m_totalCommandsSent = 0;        ///< 快捷栏已发送的指令总次数
    quint64 m_totalQuickSends = 0;          ///< 快捷发送累计发送的总字节数
    quint64 m_maxCommandLength = 0;         ///< 历史最大单条指令长度（字节数）
    quint64 m_totalHexCommands = 0;         ///< HEX模式指令触发总次数
    quint64 m_totalEditDialogOpens = 0;     ///< 编辑对话框打开总次数
    quint64 m_totalMacrosRun = 0;           ///< 宏指令运行总次数(多指令批量执行)
};

#endif // QUICKCOMMANDBAR_H

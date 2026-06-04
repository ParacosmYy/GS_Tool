/**
 * @file EdDialog2.h
 * @brief EmbedDebug统一对话框 - 替代QMessageBox的自定义弹窗组件
 *
 * 职责:
 *   1. 提供统一的确认/警告/错误对话框，替代原生QMessageBox
 *   2. 支持标准按钮和自定义按钮
 *   3. 支持图标设置和自定义内容widget
 *   4. 提供"记住选择"复选框功能
 *
 * 设计约束: 参见 05-ui-standard §十六 — 全局统一使用EdDialog替代QMessageBox
 */

#pragma once
#include <QDialog>
#include <QString>
#include <QMap>

/**
 * @brief EmbedDebug统一对话框
 *
 * 提供标题、消息、按钮、图标和可选的"记住选择"复选框。
 * 支持嵌入自定义QWidget作为内容区域，满足复杂对话框需求。
 */
class EdDialog : public QDialog {
    Q_OBJECT
public:
    /** @brief 标准按钮枚举 */
    enum StandardButton { Ok, Cancel, Yes, No, Apply, Close };

    /**
     * @brief 构造对话框
     * @param parent 父widget
     */
    explicit EdDialog(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~EdDialog() override;

    /**
     * @brief 设置对话框标题
     * @param title 标题文本
     */
    void setTitle(const QString &title);

    /**
     * @brief 设置消息正文
     * @param msg 消息文本
     */
    void setMessage(const QString &msg);

    /**
     * @brief 添加标准按钮
     * @param btn 标准按钮类型
     */
    void addButton(StandardButton btn);

    /**
     * @brief 添加自定义按钮
     * @param label 按钮文字
     * @param roleId 按钮角色标识（用于resultRole()返回值）
     */
    void addCustomButton(const QString &label, int roleId);

    /**
     * @brief 设置对话框图标
     * @param iconName 图标名称（传递给IconManager）
     */
    void setIcon(const QString &iconName);

    /**
     * @brief 设置自定义内容widget（替换默认消息区域）
     * @param widget 内容widget指针（对话框不获取所有权）
     */
    void setContentWidget(QWidget *widget);

    /**
     * @brief 获取对话框结果角色
     * @return 点击按钮对应的roleId或标准按钮枚举值
     */
    int resultRole() const;

    /**
     * @brief 添加"记住选择"复选框
     * @param key 持久化键名（用于区分不同对话框的记住状态）
     * @param label 复选框标签文本
     */
    void setRememberOption(const QString &key, const QString &label);

    /**
     * @brief 查询指定键的"记住选择"是否被勾选
     * @param key 持久化键名
     * @return true表示用户勾选了记住
     */
    bool isRememberChecked(const QString &key) const;

signals:
    /** @brief 按钮被点击 @param role 按钮角色标识 */
    void buttonClicked(int role);

private:
    /** @brief 初始化对话框UI布局 */
    void setupUi();

    /** @brief "记住选择"条目结构体 */
    struct RememberEntry {
        QString key;            ///< 持久化键名
        QString label;          ///< 复选框标签
        bool checked = false;   ///< 是否勾选
    };

    QMap<QString, RememberEntry> m_remember; ///< 记住选择条目表（按键名索引）
    int m_resultRole = 0;                    ///< 对话框结果角色值

    // ---- 统计计数器(static inline，跨实例累积) ----
    static inline quint64 s_totalOpens = 0;        ///< 对话框总打开次数
    static inline quint64 s_totalButtonPresses = 0; ///< 总按钮点击次数
    static inline quint64 s_totalRememberSets = 0;  ///< 总记住选择设置次数

public:
    /** @brief 获取对话框总打开次数 @return 累计打开次数 */
    static quint64 totalOpens() { return s_totalOpens; }
    /** @brief 获取总按钮点击次数 @return 累计按钮点击次数 */
    static quint64 totalButtonPresses() { return s_totalButtonPresses; }
    /** @brief 获取总记住选择设置次数 @return 累计设置次数 */
    static quint64 totalRememberSets() { return s_totalRememberSets; }
    /** @brief 重置对话框统计计数器 */
    static void resetDialog2Statistics() { s_totalOpens = 0; s_totalButtonPresses = 0; s_totalRememberSets = 0; }
};

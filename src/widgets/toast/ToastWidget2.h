/**
 * @file ToastWidget2.h
 * @brief Toast通知组件 - 提供轻量级的临时消息提示
 *
 * 职责:
 *   1. 在指定角落显示分级消息（信息/成功/警告/错误）
 *   2. 自动定时消失，点击可立即关闭
 *   3. 不同级别使用不同颜色和持续时间
 */

#pragma once
#include <QWidget>
#include <QString>
#include <QTimer>
#include <QLabel>

/**
 * @brief Toast通知组件
 *
 * 提供Info/Success/Warning/Error四种级别的临时消息提示，
 * 显示在父窗口的指定角落，超时后自动消失。
 * 用法: showMessage("保存成功", Success, 3000);
 */
class ToastWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief Toast消息级别 */
    enum ToastType { Info, Success, Warning, Error };
    Q_ENUM(ToastType)

    /**
     * @brief 构造Toast组件
     * @param parent 父widget
     */
    explicit ToastWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~ToastWidget() override;

    /**
     * @brief 显示Toast消息
     * @param text 消息文本
     * @param type 消息级别，默认Info
     * @param durationMs 显示持续时间（毫秒），默认3000
     */
    void showMessage(const QString &text, ToastType type = Info, int durationMs = 3000);

    /**
     * @brief 显示成功级别消息
     * @param text 消息文本
     * @param durationMs 显示持续时间（毫秒），默认3000
     */
    void showSuccess(const QString &text, int durationMs = 3000);

    /**
     * @brief 显示警告级别消息
     * @param text 消息文本
     * @param durationMs 显示持续时间（毫秒），默认4000
     */
    void showWarning(const QString &text, int durationMs = 4000);

    /**
     * @brief 显示错误级别消息
     * @param text 消息文本
     * @param durationMs 显示持续时间（毫秒），默认5000
     */
    void showError(const QString &text, int durationMs = 5000);

    /** @brief 立即关闭Toast */
    void dismiss();

    /**
     * @brief 查询Toast是否可见
     * @return true表示当前可见
     */
    bool isVisible() const;

    /**
     * @brief 设置Toast显示位置
     * @param corner 窗口角落位置，默认右上角
     */
    void setPosition(Qt::Corner corner);

protected:
    /** @brief 自定义绘制Toast背景和边框 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 点击关闭Toast */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /** @brief 初始化UI布局 */
    void setupUi();

    /** @brief 根据消息级别更新样式 */
    void updateStyle();

    QLabel *m_label = nullptr;          ///< 消息文本标签
    QTimer *m_timer = nullptr;          ///< 自动关闭定时器
    ToastType m_type = Info;            ///< 当前消息级别
    Qt::Corner m_corner = Qt::TopRightCorner; ///< 显示位置角落

    // ---- 统计计数器 ----
    quint64 m_totalMessages = 0;         ///< 总消息显示次数
    quint64 m_totalDismisses = 0;        ///< 总关闭次数

public:
    /** @brief 获取总消息显示次数 @return 累计显示次数 */
    quint64 totalMessages() const { return m_totalMessages; }
    /** @brief 获取总关闭次数 @return 累计关闭次数 */
    quint64 totalDismisses() const { return m_totalDismisses; }
    /** @brief 重置Toast统计计数器 */
    void resetToast2Statistics() { m_totalMessages = 0; m_totalDismisses = 0; }
};

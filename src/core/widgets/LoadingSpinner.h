/**
 * @file LoadingSpinner.h
 * @brief 加载旋转指示器 — 圆弧旋转动画，用于面板数据加载中场景
 *
 * QPainter 绘制 270° 圆弧，1000ms linear 循环旋转。
 * 使用 ThemeManager::Accent 语义色。
 */
#ifndef LOADING_SPINNER_H
#define LOADING_SPINNER_H

#include <QWidget>

/**
 * @brief 加载旋转指示器
 *
 * 使用场景:
 *   - 面板数据加载中
 *   - 设备扫描中
 *   - 文件解析中
 */
class LoadingSpinner : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造旋转指示器
     * @param size 控件尺寸(px)，默认32
     * @param parent 父控件
     */
    explicit LoadingSpinner(int size = 32, QWidget* parent = nullptr);

    /** @brief 获取线条宽度 */
    int lineWidth() const;
    /** @brief 设置线条宽度 */
    void setLineWidth(int width);

    /** @brief 启动旋转动画 */
    void start();
    /** @brief 停止旋转动画 */
    void stop();
    /** @brief 动画是否正在运行 */
    bool isSpinning() const;

    // ── 统计计数器 ──

    /** @brief 获取动画启动总次数 */
    quint64 totalStarts() const { return m_totalStarts; }
    /** @brief 获取动画停止总次数 */
    quint64 totalStops() const { return m_totalStops; }
    /** @brief 获取旋转定时器触发总次数(累计滴答计数) */
    quint64 totalTicks() const { return m_totalTicks; }
    /** @brief 重置所有统计计数器 */
    void resetSpinnerStatistics();

protected:
    /** @brief 绘制旋转圆弧 */
    void paintEvent(QPaintEvent* event) override;

private:
    int m_lineWidth = 3;  ///< 弧线宽度(px)
    int m_angle = 0;      ///< 当前旋转角度(0-360)

    /** @brief 旋转定时器 */
    class QTimer* m_timer = nullptr;

    // ── 统计计数器 ──
    quint64 m_totalStarts = 0;   ///< 动画启动次数
    quint64 m_totalStops = 0;    ///< 动画停止次数
    mutable quint64 m_totalTicks = 0; ///< 旋转定时器触发总次数(累计滴答)
};

#endif // LOADING_SPINNER_H

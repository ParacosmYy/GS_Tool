/**
 * @file CircularBufferWidget.h
 * @brief 环形缓冲区可视化控件 -- 甜甜圈图展示填充率/读写指针/溢出统计
 *
 * 以环形图(甜甜圈)形式可视化缓冲区状态，包含:
 *   - 填充率弧线(强调色)
 *   - 读指针标记(绿色三角)
 *   - 写指针标记(红色三角)
 *   - 中心文本(使用率百分比)
 *   - 底部溢出计数器(警告色)
 *
 * 协作关系:
 *   - ThemeManager: 提供语义色板
 *   - 数据管道组件: 通过 setUsed/setReadPos/setWritePos 更新状态
 */

#ifndef CIRCULARBUFFERWIDGET_H
#define CIRCULARBUFFERWIDGET_H

#include <QWidget>
#include <QPaintEvent>

/**
 * @class CircularBufferWidget
 * @brief 环形缓冲区可视化控件
 *
 * 实时展示缓冲区填充率、读写指针位置和溢出计数。
 * 所有颜色通过 ThemeManager 语义色板获取，无硬编码颜色。
 */
class CircularBufferWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 运行统计数据结构体，聚合全部运行期间计数器
     */
    struct Stats {
        quint64 totalUpdates = 0;          ///< 累计状态更新次数
        quint64 totalOverflows = 0;        ///< 累计溢出标记次数
        quint64 totalCapacityChanges = 0;  ///< 累计容量变更次数
        quint64 totalPointerMoves = 0;     ///< 累计读写指针移动次数
        quint64 totalRenders = 0;          ///< 累计渲染次数(paintEvent触发)
        quint64 totalResets = 0;           ///< 累计统计重置次数
        int     peakUsed = 0;              ///< 历史峰值使用量(字节)
        double  cumulativePercent = 0.0;   ///< 累计填充率(用于计算平均值)
    };

    /**
     * @brief 构造函数
     * @param parent 父Widget指针
     */
    explicit CircularBufferWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~CircularBufferWidget() override;

    // ==================== 状态设置接口 ====================

    /**
     * @brief 设置缓冲区总容量
     * @param capacity 缓冲区容量(字节)，必须 > 0
     */
    void setCapacity(int capacity);

    /**
     * @brief 设置当前已使用字节数
     * @param used 已使用字节数，自动钳位到 [0, capacity]
     */
    void setUsed(int used);

    /**
     * @brief 设置读指针位置
     * @param pos 读指针在缓冲区中的偏移量
     */
    void setReadPos(int pos);

    /**
     * @brief 设置写指针位置
     * @param pos 写指针在缓冲区中的偏移量
     */
    void setWritePos(int pos);

    /**
     * @brief 标记一次溢出事件，溢出计数 +1
     */
    void markOverflow();

    /**
     * @brief 重置溢出计数器为零
     */
    void resetOverflowCount();

    // ==================== 状态查询接口 ====================

    /** @brief 获取缓冲区容量 @return 容量(字节) */
    int capacity() const;

    /** @brief 获取当前使用量 @return 已用字节数 */
    int used() const;

    /** @brief 获取读指针位置 @return 读偏移量 */
    int readPos() const;

    /** @brief 获取写指针位置 @return 写偏移量 */
    int writePos() const;

    /** @brief 获取当前填充率百分比 @return 0.0 ~ 100.0 */
    double fillPercent() const;

    /** @brief 获取累计溢出次数 @return 溢出计数 */
    quint64 overflowCount() const;

    // ==================== 统计接口 ====================

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

protected:
    /** @brief 绘制环形缓冲区可视化图形 @param event 绘制事件 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 建议最小尺寸 @return 120x120像素 */
    QSize minimumSizeHint() const override;

private:
    int m_capacity = 1024;       ///< 缓冲区总容量(字节)
    int m_used = 0;              ///< 当前已使用字节数
    int m_readPos = 0;           ///< 读指针位置
    int m_writePos = 0;          ///< 写指针位置
    quint64 m_overflowCount = 0; ///< 累计溢出计数

    Stats m_stats;               ///< 聚合统计结构体
};

#endif // CIRCULARBUFFERWIDGET_H

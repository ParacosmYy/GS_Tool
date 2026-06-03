/**
 * @file CanBusMonitor.h
 * @brief CAN总线监控面板 — 以表格形式实时显示CAN总线帧流和信号解码
 *
 * 职责: 接收并展示CAN帧列表，显示帧ID、数据、类型、时间戳，
 * 支持清空、帧计数统计、颜色编码、自动滚动、帧过滤、
 * DBC信号解码显示和统计面板。
 */
#ifndef CANBUSMONITOR_H
#define CANBUSMONITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QMap>
#include <QElapsedTimer>
#include "connection/can/CanFrameParser.h"

/**
 * @brief CAN总线帧监控面板
 *
 * 以表格形式实时展示收到的CAN帧，包含帧ID、DLC、数据、时间戳等列。
 * 支持颜色编码(标准帧/扩展帧/RTR帧/FD帧)、10000行上限、
 * 帧ID过滤、DBC信号解码显示和统计面板。
 */
class CanBusMonitor : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造CAN总线监控面板
     * @param parent 父控件
     */
    explicit CanBusMonitor(QWidget* parent = nullptr);

    /**
     * @brief 添加一帧到监控列表
     * @param frame CAN帧数据
     */
    void addFrame(const CanFrame& frame);

    /** @brief 清空所有帧记录 */
    void clearFrames();

    /**
     * @brief 获取当前帧总数
     * @return 已记录的帧数量
     */
    int frameCount() const;

    /**
     * @brief 获取唯一帧ID数量
     * @return 不同帧ID的数量
     */
    int uniqueFrameIdCount() const;

    /**
     * @brief 获取统计摘要文本
     * @return 帧统计信息的格式化字符串
     */
    QString statisticsSummary() const;

    /**
     * @brief 设置帧ID过滤器（只显示匹配的帧）
     * @param filterText 帧ID过滤文本（如 "0x123"），空字符串清除过滤
     */
    void setFrameIdFilter(const QString& filterText);

    /**
     * @brief 设置DBC解析器用于信号解码显示
     * @param parser DBC解析器指针(不转移所有权)
     */
    void setDbcParser(class DbcParser* parser);

    /** @brief 获取累计监控帧总数 */
    quint64 totalFramesMonitored() const;

    /** @brief 获取累计错误次数 */
    quint64 totalErrors() const;

    /** @brief 获取标准帧数量 */
    quint64 totalStandardFrames() const { return m_totalStandardFrames; }

    /** @brief 获取扩展帧数量 */
    quint64 totalExtendedFrames() const { return m_totalExtendedFrames; }

    /** @brief 获取CAN-FD帧数量 */
    quint64 totalFdFrames() const { return m_totalFdFrames; }

    /** @brief 获取RTR帧数量 */
    quint64 totalRtrFrames() const { return m_totalRtrFrames; }

    /** @brief 获取总接收字节数 */
    quint64 totalBytesReceived() const { return m_totalBytesReceived; }

    /** @brief 获取帧率(fps) */
    double frameRate() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private:
    /**
     * @brief 根据帧类型获取行背景色
     * @param frame CAN帧
     * @return 背景QBrush
     */
    QBrush rowBrush(const CanFrame& frame) const;

    /** @brief 更新统计面板标签文本 */
    void updateStatsDisplay();

    /** @brief 帧列表表格控件 */
    QTableWidget* m_frameTable;

    /** @brief 信号解码表格控件 */
    QTableWidget* m_signalTable;

    /** @brief 帧计数标签 */
    QLabel* m_countLabel;

    /** @brief 清空按钮 */
    QPushButton* m_clearBtn;

    /** @brief 自动滚动复选框 */
    QCheckBox* m_autoScrollCheck;

    /** @brief 帧ID过滤输入框 */
    QLineEdit* m_filterEdit;

    /** @brief 帧类型过滤下拉框 */
    QComboBox* m_typeFilterCombo;

    /** @brief 统计信息标签 */
    QLabel* m_statsLabel;

    /** @brief 帧计数器 */
    int m_frameCount = 0;

    /** @brief 最大显示行数 */
    static constexpr int kMaxRows = 10000;

    /** @brief 帧ID频率统计 */
    QMap<quint32, int> m_idFrequency;

    /** @brief 当前帧ID过滤文本 */
    QString m_frameIdFilter;

    /** @brief DBC解析器(不拥有所有权) */
    class DbcParser* m_dbcParser = nullptr;

    /** @brief 帧率计算计时器 */
    QElapsedTimer m_rateTimer;

    /** @brief 帧率计算用帧数 */
    int m_rateFrameCount = 0;

    // ---- 统计计数器 ----
    quint64 m_totalFramesMonitored = 0;         ///< 累计监控帧总数
    quint64 m_totalErrors = 0;                  ///< 累计错误次数
    quint64 m_totalStandardFrames = 0;          ///< 标准帧计数
    quint64 m_totalExtendedFrames = 0;          ///< 扩展帧计数
    quint64 m_totalFdFrames = 0;                ///< CAN-FD帧计数
    quint64 m_totalRtrFrames = 0;               ///< RTR帧计数
    quint64 m_totalBytesReceived = 0;           ///< 总接收字节数
};

#endif // CANBUSMONITOR_H

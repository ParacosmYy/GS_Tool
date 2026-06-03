/**
 * @file CanBusMonitor.h
 * @brief CAN总线监控面板 — 以表格形式实时显示CAN总线帧流
 *
 * 职责: 接收并展示CAN帧列表，显示帧ID、数据、类型、时间戳，
 * 支持清空、帧计数统计、颜色编码和自动滚动。
 */
#ifndef CANBUSMONITOR_H
#define CANBUSMONITOR_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QMap>
#include "connection/can/CanFrameParser.h"

/**
 * @brief CAN总线帧监控面板
 *
 * 以表格形式实时展示收到的CAN帧，包含帧ID、DLC、数据、时间戳等列。
 * 支持颜色编码(标准帧白色、扩展帧浅蓝、RTR帧黄色)和10000行上限。
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

private:
    /**
     * @brief 根据帧类型获取行背景色
     * @param frame CAN帧
     * @return 背景QBrush
     */
    QBrush rowBrush(const CanFrame& frame) const;

    /** @brief 帧列表表格控件 */
    QTableWidget* m_frameTable;

    /** @brief 帧计数标签 */
    QLabel* m_countLabel;

    /** @brief 清空按钮 */
    QPushButton* m_clearBtn;

    /** @brief 自动滚动复选框 */
    QCheckBox* m_autoScrollCheck;

    /** @brief 帧计数器 */
    int m_frameCount = 0;

    /** @brief 最大显示行数 */
    static constexpr int kMaxRows = 10000;

    /** @brief 帧ID频率统计 */
    QMap<quint32, int> m_idFrequency;

    /** @brief 当前帧ID过滤文本 */
    QString m_frameIdFilter;
};

#endif // CANBUSMONITOR_H

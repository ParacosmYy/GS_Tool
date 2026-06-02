/**
 * @file CanBusMonitor.h
 * @brief CAN总线监控面板 — 以表格形式实时显示CAN总线帧流
 *
 * 职责: 接收并展示CAN帧列表，显示帧ID、数据、类型、时间戳，
 * 支持清空和帧计数统计。
 */
#ifndef CANBUSMONITOR_H
#define CANBUSMONITOR_H

#include <QWidget>
#include <QTableWidget>
#include "connection/can/CanFrameParser.h"

/**
 * @brief CAN总线帧监控面板
 *
 * 以表格形式实时展示收到的CAN帧，包含帧ID、DLC、数据、时间戳等列。
 * 通过addFrame()接收新帧并追加到表格。
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

private:
    /** @brief 帧列表表格控件 */
    QTableWidget* m_frameTable;

    /** @brief 帧计数器 */
    int m_frameCount = 0;
};

#endif // CANBUSMONITOR_H

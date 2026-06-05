/**
 * @file EyeTypes.h
 * @brief 眼图分析器基础数据类型 — 参数/测量结果/掩模定义
 *
 * 设计: 纯数据结构(无行为)、与QImage/QWidget解耦、全部字段零初始化
 * 协作: EyeDiagramEngine(消费EyeParams/产生EyeMeasurement) / EyeDiagramWidget(显示)
 */

#ifndef EYETYPES_H
#define EYETYPES_H

#include <QString>
#include <QVector>
#include <QPointF>

/**
 * @brief 眼图生成参数 — 配置信号源特性
 *
 * bitRate/sampleRate决定UI周期长度；riseTime/fallTime/jitter/noise影响眼图形态。
 * 所有时间单位为秒，所有速率单位为Hz。
 */
struct EyeParams {
    double bitRate     = 1.0e6;  ///< 比特率(Hz)，默认1MHz
    double sampleRate  = 10.0e6; ///< 采样率(Hz)，默认10MHz
    double riseTime    = 0.0;    ///< 信号上升时间(秒)，0=使用默认值
    double fallTime    = 0.0;    ///< 信号下降时间(秒)，0=使用默认值
    double jitter      = 0.0;    ///< 时间抖动标准差(秒)
    double noise       = 0.0;    ///< 幅度噪声标准差(V)
};

/**
 * @brief 眼图测量结果 — 信号质量关键指标
 *
 * 由EyeDiagramEngine::measure()计算返回，涵盖眼高/眼宽/抖动/SNR/BER/Q因子。
 */
struct EyeMeasurement {
    double eyeHeight         = 0.0;  ///< 眼高(V)，逻辑1与逻辑0在最佳采样点的差值
    double eyeWidth          = 0.0;  ///< 眼宽(秒)，眼图开口的最大水平宽度
    double eyeAmplitude      = 0.0;  ///< 眼幅度(V)，峰峰值
    double jitterRms         = 0.0;  ///< RMS抖动(秒)
    double jitterPp          = 0.0;  ///< 峰峰值抖动(秒)
    double riseTime          = 0.0;  ///< 实测上升时间(秒)，20%~80%
    double fallTime          = 0.0;  ///< 实测下降时间(秒)，80%~20%
    double signalToNoiseRatio = 0.0; ///< 信噪比(dB)
    double bitErrorRate      = 0.0;  ///< 估计误码率
    double qualityFactor     = 0.0;  ///< Q因子，信号质量综合评分
};

/**
 * @brief 眼图掩模 — 合规测试区域定义
 *
 * 上/下边界为QPointF集合(x=时间偏移秒, y=电压V)。掩模叠加在眼图上，
 * 若信号轨迹穿过掩模区域则记为一次hit。
 */
struct EyeMask {
    QString name;                ///< 掩模名称
    QVector<QPointF> upperBoundary; ///< 上边界折线点集
    QVector<QPointF> lowerBoundary; ///< 下边界折线点集
    double hitRate = 0.0;        ///< 掩模违规率(0.0~1.0)
};

#endif // EYETYPES_H

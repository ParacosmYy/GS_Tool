/**
 * @file EyeDiagramEngine.h
 * @brief 眼图分析引擎 — 数字信号质量分析与眼图渲染
 *
 * 设计: QObject子类、接收采样数据→时钟恢复→眼图叠加→测量/掩模测试
 * 协作: EyeDiagramWidget(显示renderEyeDiagram输出的QImage) / EyeTypes(数据结构)
 *
 * 算法:
 *   1. feedSamples()累积原始采样
 *   2. recoverClock()从数据边沿提取时钟对齐点
 *   3. 按UI周期切片并叠加到2D直方图
 *   4. renderEyeDiagram()将直方图映射为彩色QImage
 *   5. measure()在直方图上执行眼高/眼宽/抖动/SNR/BER计算
 *   6. setMask()/maskHitRate()执行掩模合规测试
 */

#ifndef EYEDIAGRAMENGINE_H
#define EYEDIAGRAMENGINE_H

#include <QObject>
#include <QImage>
#include <QVector>

#include "chart/eye/EyeTypes.h"

/** @brief 眼图分析引擎 — 从数字信号采样数据生成眼图、执行信号质量测量与掩模测试 */
class EyeDiagramEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 构造眼图引擎 @param parent 父对象 */
    explicit EyeDiagramEngine(QObject* parent = nullptr);

    // ---- 参数配置 ----

    /** @brief 设置眼图生成参数(比特率/采样率/抖动等) @param params 参数集 */
    void setParameters(const EyeParams& params);

    /** @brief 获取当前眼图参数 @return 参数集 */
    EyeParams parameters() const;

    // ---- 数据输入 ----

    /** @brief 输入一批采样数据用于眼图构建 @param samples 电压采样序列(V) */
    void feedSamples(const QVector<double>& samples);

    // ---- 渲染 ----

    /** @brief 渲染眼图为QImage @param width 图像宽度(像素) @param height 图像高度(像素) @return 眼图彩色图像 */
    QImage renderEyeDiagram(int width, int height) const;

    // ---- 测量 ----

    /** @brief 执行眼图测量(眼高/眼宽/抖动/SNR/BER/Q因子) @return 测量结果 */
    EyeMeasurement measure() const;

    // ---- 掩模测试 ----

    /** @brief 设置眼图掩模 @param mask 掩模定义(上/下边界) */
    void setMask(const EyeMask& mask);

    /** @brief 获取当前掩模 @return 掩模定义 */
    EyeMask mask() const;

    /** @brief 获取掩模违规率 @return 违规率(0.0~1.0) */
    double maskHitRate() const;

    /** @brief 从文件加载掩模定义(JSON格式) @param filePath 文件路径 @return 是否加载成功 */
    bool loadMask(const QString& filePath);

    // ---- 统计 ----

    /** @brief 获取累计处理的采样点数 */
    quint64 totalSamplesProcessed() const;

    /** @brief 获取累计处理的比特数 */
    quint64 totalBitsProcessed() const;

    /** @brief 获取累计叠加的眼图层数 */
    quint64 totalOverlays() const;

    /** @brief 获取累计测量次数 */
    quint64 totalMeasurements() const;

    /** @brief 获取累计掩模违规次数 */
    quint64 totalMaskHits() const;

    /** @brief 获取累计掩模测试次数 */
    quint64 totalMaskTests() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 眼图数据更新信号(新数据叠加后发射) */
    void diagramUpdated();

    /** @brief 测量完成信号 @param m 测量结果 */
    void measurementComplete(const EyeMeasurement& m);

    /** @brief 掩模违规率更新信号 @param rate 新违规率 */
    void maskHitRateUpdated(double rate);

private:
    /** @brief 从采样数据中恢复时钟对齐点 @param samples 采样序列 @return 边沿位置索引集 */
    QVector<int> recoverClock(const QVector<double>& samples) const;

    /** @brief 将密度值映射为颜色(蓝→绿→黄→红) @param density 归一化密度(0~1) @return 颜色 */
    QColor densityColor(double density) const;

    /** @brief 在直方图上执行掩模测试 @return 违规的采样点数 */
    quint64 testMaskOnHistogram() const;

    EyeParams m_params;                      ///< 眼图参数
    EyeMask   m_mask;                        ///< 掩模定义

    QVector<double> m_sampleBuffer;          ///< 待处理采样缓冲区
    QVector<QVector<quint64>> m_histogram;   ///< 2D密度直方图[col][row]
    int m_histCols = 0;                      ///< 直方图列数(UI周期内采样点数)
    int m_histRows = 0;                      ///< 直方图行数(电压量化级)

    double m_voltageMin = -1.0;              ///< 直方图电压下限(V)
    double m_voltageMax = 1.0;               ///< 直方图电压上限(V)
    quint64 m_maxDensity = 1;                ///< 直方图峰值密度(用于归一化)

    // 统计计数器
    quint64 m_totalSamplesProcessed = 0;     ///< 累计采样点数
    quint64 m_totalBitsProcessed   = 0;      ///< 累计比特数
    quint64 m_totalOverlays        = 0;      ///< 累计叠加层数
    quint64 m_totalMeasurements    = 0;      ///< 累计测量次数
    quint64 m_totalMaskHits        = 0;      ///< 累计掩模违规
    quint64 m_totalMaskTests       = 0;      ///< 累计掩模测试
};

#endif // EYEDIAGRAMENGINE_H

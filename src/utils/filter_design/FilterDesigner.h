/**
 * @file FilterDesigner.h
 * @brief 数字滤波器设计引擎 — IIR/FIR滤波器系数计算与频率响应分析
 *
 * 设计: QObject子类、接收FilterParams→设计算法→输出FilterCoeffs/FilterResponse
 * 协作: FilterTypes(数据结构) / FilterDesignerWidget(可视化控件)
 *
 * 算法:
 *   1. designIIR() — Butterworth/Chebyshev/Bessel 极点放置 + 双线性变换
 *   2. designFIR()  — 窗函数sinc法 (Hamming/Hanning/Blackman)
 *   3. frequencyResponse() — 在单位圆上计算 H(z) 的幅度(dB)和相位(度)
 */

#ifndef FILTERDESIGNER_H
#define FILTERDESIGNER_H

#include <QObject>
#include <QVector>
#include <complex>

#include "utils/filter_design/FilterTypes.h"

/** @brief 数字滤波器设计引擎 — IIR/FIR滤波器系数计算与频率响应分析 */
class FilterDesigner : public QObject {
    Q_OBJECT

public:
    /** @brief 构造滤波器设计器 @param parent 父对象 */
    explicit FilterDesigner(QObject* parent = nullptr);

    // ---- 设计接口 ----

    /** @brief 设计IIR滤波器(Butterworth/Chebyshev/Bessel)
     *  @param params 滤波器参数(必须为IIR族)
     *  @return 滤波器系数(b/a)，参数无效时返回空 */
    FilterCoeffs designIIR(const FilterParams& params);

    /** @brief 设计FIR滤波器(窗函数sinc法)
     *  @param params 滤波器参数(family须为FIR)
     *  @return 滤波器系数(b, a为空)，参数无效时返回空 */
    FilterCoeffs designFIR(const FilterParams& params);

    // ---- 频率响应 ----

    /** @brief 计算频率响应(幅度dB + 相位度)
     *  @param coeffs 滤波器系数
     *  @param sampleRate 采样率(Hz)
     *  @param numPoints 频率点数
     *  @return 频率响应(freq/mag/phase) */
    FilterResponse frequencyResponse(const FilterCoeffs& coeffs,
                                     double sampleRate,
                                     int numPoints = 512) const;

    // ---- 统计 ----

    /** @brief 获取累计设计IIR滤波器次数 */
    quint64 totalIIRDesigns() const;

    /** @brief 获取累计设计FIR滤波器次数 */
    quint64 totalFIRDesigns() const;

    /** @brief 获取累计计算频率响应次数 */
    quint64 totalResponsesComputed() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 滤波器设计完成信号 @param coeffs 设计结果系数 */
    void designComplete(const FilterCoeffs& coeffs);

    /** @brief 频率响应计算完成信号 @param response 响应数据 */
    void responseReady(const FilterResponse& response);

private:
    // ---- IIR 设计子算法 ----

    /** @brief Butterworth极点放置 + 双线性变换
     *  @param order 阶数 @param Wn 归一化截止频率(0~1) @param type 滤波器类型
     *  @return (b, a)系数对 */
    QPair<QVector<double>, QVector<double>> butterworth(int order, double Wn,
                                                        FilterType type) const;

    /** @brief Chebyshev Type I 极点放置 + 双线性变换
     *  @param order 阶数 @param Wn 归一化截止 @param rippleDb 波纹(dB) @param type 类型
     *  @return (b, a)系数对 */
    QPair<QVector<double>, QVector<double>> chebyshev1(int order, double Wn,
                                                       double rippleDb,
                                                       FilterType type) const;

    /** @brief 将模拟极点经双线性变换转为数字系数
     *  @param poles 模拟极点 @param Wn 归一化频率
     *  @return (b, a)系数对 */
    QPair<QVector<double>, QVector<double>> bilinearTransform(
        const QVector<std::complex<double>>& poles, double Wn) const;

    /** @brief 二阶节级联相乘得到最终系数
     *  @param sections 各二阶节(b0,b1,b2,a0,a1,a2)
     *  @return 合并后的(b, a) */
    QPair<QVector<double>, QVector<double>> cascadeSections(
        const QVector<QVector<double>>& sections) const;

    // ---- FIR 辅助 ----

    /** @brief 生成窗函数系数 @param length 窗长 @param type 窗类型 @return 窗系数 */
    QVector<double> generateWindow(int length, WindowType type) const;

    /** @brief sinc函数 sin(pi*x)/(pi*x) @param x 输入 @return sinc值 */
    static double sinc(double x);

    // ---- 统计计数器 ----
    quint64 m_totalIIRDesigns      = 0;  ///< 累计IIR设计次数
    quint64 m_totalFIRDesigns      = 0;  ///< 累计FIR设计次数
    quint64 m_totalResponsesComputed = 0; ///< 累计频率响应计算次数
};

#endif // FILTERDESIGNER_H

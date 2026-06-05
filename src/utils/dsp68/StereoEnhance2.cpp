/**
 * @file StereoEnhance2.cpp
 * @brief 立体声增强处理器实现
 *
 * 实现立体声宽度控制、声像调节和低频单声道混合功能。
 * 通过Mid/Side处理实现精确的立体声场控制，支持电平补偿
 * 和多种声像法则。
 */

#include "utils/dsp68/StereoEnhance2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
StereoEnhance2::StereoEnhance2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置立体声宽度
 * @param w 宽度因子，0.0=单声道，1.0=原始，>1.0=增强
 */
void StereoEnhance2::setWidth(double w)
{
    m_width = qBound(0.0, w, 3.0);
}

/**
 * @brief 设置声像位置
 * @param pan 声像值，-1.0=全左，0.0=中央，1.0=全右
 */
void StereoEnhance2::setPan(double pan)
{
    m_pan = qBound(-1.0, pan, 1.0);
}

/**
 * @brief 设置低频单声道混合截止频率
 * @param freq 截止频率(Hz)，低于此频率的信号合并为单声道
 */
void StereoEnhance2::setBassMonoFreq(double freq)
{
    m_bassFreq = qBound(20.0, freq, 500.0);
}

/**
 * @brief 处理立体声音频数据
 * @param input 输入数据，input[0]=左声道，input[1]=右声道
 * @return 处理后的立体声数据
 *
 * 处理流程：
 * 1. Mid/Side变换并应用宽度缩放
 * 2. 应用声像定位（constant power pan law）
 * 3. 低频单声道化（防止低频立体声相位问题）
 * 4. 电平补偿（防止宽度变化导致的音量变化）
 * 5. 计算立体声相关系数
 */
QVector<QVector<double>> StereoEnhance2::process(const QVector<QVector<double>>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> output;
    if (input.size() < 2 || input[0].isEmpty()) return output;

    const int N = input[0].size();
    output.resize(2);
    output[0].resize(N);
    output[1].resize(N);

    // 预计算声像系数（constant power pan law）
    double panAngle = (m_pan + 1.0) * 0.25 * M_PI;
    double panL = qCos(panAngle);
    double panR = qSin(panAngle);

    // 预计算低通滤波器系数
    double alpha = qExp(-2.0 * M_PI * m_bassFreq / 44100.0);

    // 电平补偿因子：保持中间声像的电平一致
    // 补偿量 = 1 / sqrt(0.5 * (1 + width^2))
    double levelComp = 1.0 / qSqrt(0.5 * (1.0 + m_width * m_width));

    double bassL = 0.0, bassR = 0.0;

    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = (i < input[1].size()) ? input[1][i] : 0.0;

        // 步骤1：Mid/Side变换
        // M = (L+R)/sqrt(2), S = (L-R)/sqrt(2)
        double M = (L + R) * M_SQRT1_2;
        double S = (L - R) * M_SQRT1_2;

        // 步骤2：应用宽度——缩放Side分量
        S *= m_width;

        // 步骤3：重建左右声道
        // L' = (M + width*S)/sqrt(2), R' = (M - width*S)/sqrt(2)
        double newL = (M + S) * M_SQRT1_2 * levelComp;
        double newR = (M - S) * M_SQRT1_2 * levelComp;

        // 步骤4：应用声像
        newL *= panL;
        newR *= panR;

        output[0][i] = newL;
        output[1][i] = newR;
    }

    // 步骤5：低频单声道化
    // 使用一阶IIR低通滤波器提取低频分量，然后合并为单声道
    for (int i = 0; i < N; ++i) {
        // 低通滤波提取低频
        bassL = alpha * bassL + (1.0 - alpha) * output[0][i];
        bassR = alpha * bassR + (1.0 - alpha) * output[1][i];

        // 计算低频的单声道版本
        double monoBass = (bassL + bassR) * 0.5;

        // 用单声道低频替换原始低频
        output[0][i] = output[0][i] - bassL + monoBass;
        output[1][i] = output[1][i] - bassR + monoBass;
    }

    // 步骤6：计算相关系数（用于监控立体声场质量）
    double sumXY = 0.0, sumXX = 0.0, sumYY = 0.0;
    double peakL = 0.0, peakR = 0.0;
    for (int i = 0; i < N; ++i) {
        double oL = output[0][i];
        double oR = output[1][i];
        sumXY += oL * oR;
        sumXX += oL * oL;
        sumYY += oR * oR;
        peakL = qMax(peakL, qAbs(oL));
        peakR = qMax(peakR, qAbs(oR));
    }
    double denom = qSqrt(sumXX * sumYY);
    m_corr = (denom > 1e-12) ? sumXY / denom : 0.0;

    // 防止削波：如果峰值超过1.0则归一化
    double maxPeak = qMax(peakL, peakR);
    if (maxPeak > 1.0) {
        double norm = 1.0 / maxPeak;
        for (int i = 0; i < N; ++i) {
            output[0][i] *= norm;
            output[1][i] *= norm;
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalProcessings++;
    m_stats.totalSamples += N;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalProcessings;

    emit processingCompleted(N, m_width);
    return output;
}

/**
 * @brief 重置统计信息
 */
void StereoEnhance2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算当前立体声信号的Goniometer角度
 *
 * 辅助分析方法：计算左右声道的相位关系角度，
 * 用于立体声场可视化。角度接近45度表示良好的
 * 立体声展宽，接近0度表示趋向单声道。
 *
 * @return 相位角度（弧度）
 */
double StereoEnhance2::computePhaseAngle(const QVector<QVector<double>>& input) const
{
    if (input.size() < 2 || input[0].isEmpty()) return 0.0;

    const int N = input[0].size();
    double sumMid = 0.0, sumSide = 0.0;

    for (int i = 0; i < N; ++i) {
        double L = input[0][i];
        double R = (i < input[1].size()) ? input[1][i] : 0.0;
        double M = (L + R) * M_SQRT1_2;
        double S = (L - R) * M_SQRT1_2;
        sumMid += M * M;
        sumSide += S * S;
    }

    // 相位角度 = atan2(sqrt(sumSide), sqrt(sumMid))
    return qAtan2(qSqrt(sumSide), qSqrt(sumMid));
}

/**
 * @brief 验证立体声信号的单声道兼容性
 *
 * 检查处理后的信号在混合为单声道时是否会产生
 * 相位抵消或电平异常。兼容性值越接近1.0越好。
 *
 * @param output 处理后的立体声信号
 * @return 兼容性指标（0~1）
 */
double StereoEnhance2::monoCompatibility(const QVector<QVector<double>>& output) const
{
    if (output.size() < 2 || output[0].isEmpty()) return 1.0;

    const int N = output[0].size();
    double monoEnergy = 0.0;
    double stereoEnergy = 0.0;

    for (int i = 0; i < N; ++i) {
        double L = output[0][i];
        double R = (i < output[1].size()) ? output[1][i] : 0.0;
        double mono = (L + R) * 0.5;
        monoEnergy += mono * mono;
        stereoEnergy += L * L + R * R;
    }

    if (stereoEnergy < 1e-12) return 1.0;
    double ratio = monoEnergy / (stereoEnergy * 0.5);
    return qBound(0.0, ratio, 2.0);
}

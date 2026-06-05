/**
 * @file DataWindowManager.h
 * @brief 数据窗函数引擎 -- 为FFT/频谱分析提供7种标准窗函数系数生成与数据加窗
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持 Rectangular/Hanning/Hamming/Blackman/Kaiser/FlatTop/Gaussian 七种窗函数,
 * 提供系数生成、数据加窗、相干增益计算等接口,
 * 并维护累计运行统计信息。
 * 线程安全: 本类为可重入,但非线程安全;多线程环境需外部同步。
 */

#ifndef DATAWINDOWMANAGER_H
#define DATAWINDOWMANAGER_H

#include <QObject>
#include <QVector>

/**
 * @class DataWindowManager
 * @brief 数据窗函数引擎,为FFT/频谱分析提供窗函数系数生成与数据加窗
 *
 * 窗函数用于减少频谱泄漏,提高频域分析精度。本引擎支持7种常用窗函数,
 * 可独立于频谱分析模块使用,也可与 FftEngine / SpectrumAnalyzer 协同工作。
 */
class DataWindowManager : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数类型枚举 */
    enum class WindowType {
        Rectangular = 0,  ///< 矩形窗: 所有系数为1.0,无频谱泄漏抑制
        Hanning     = 1,  ///< 汉宁窗: 余弦升余弦,旁瓣-31dB
        Hamming     = 2,  ///< 海明窗: 优化升余弦,旁瓣-42dB
        Blackman    = 3,  ///< 布莱克曼窗: 三项余弦,旁瓣-58dB
        Kaiser      = 4,  ///< 凯塞窗: 可调旁瓣,通过beta参数控制
        FlatTop     = 5,  ///< 平顶窗: 最大幅度精度,适用于校准
        Gaussian    = 6   ///< 高斯窗: 可调宽度,通过sigma参数控制
    };
    Q_ENUM(WindowType)

    /** @brief 累计运行统计信息 */
    struct Stats {
        quint64 totalWindowsApplied   = 0;   ///< 累计加窗应用次数
        quint64 totalPointsProcessed  = 0;   ///< 累计处理数据点数
        double  peakCoherentGain      = 0.0; ///< 历史最大相干增益值
        quint64 totalWindowGenerations = 0;  ///< 累计窗系数生成次数
    };

    /**
     * @brief 构造窗函数引擎
     * @param parent 父对象,纳入QObject父子树自动管理生命周期
     *
     * 默认配置: Rectangular窗, 窗长度256点。
     */
    explicit DataWindowManager(QObject *parent = nullptr);

    // ── 配置接口 ──

    /** @brief 设置窗函数类型 @param type 窗函数类型 */
    void setWindowType(WindowType type);

    /** @brief 设置窗长度(采样点数) @param size 窗长度,必须 > 0 */
    void setWindowSize(int size);

    // ── 核心计算接口 ──

    /**
     * @brief 根据当前类型和长度生成窗函数系数
     * @return 窗系数向量;参数无效时返回空向量
     *
     * 生成后会更新 totalWindowGenerations 统计。
     * 系数保存在内部缓存中,可通过 windowCoefficients() 获取。
     */
    QVector<double> generateWindow();

    /**
     * @brief 对输入数据应用当前窗函数(逐元素相乘)
     * @param data 输入时域数据
     * @return 加窗后的数据;大小不匹配或无效输入返回空向量
     *
     * 若数据长度与窗长度不匹配,自动重新生成窗系数。
     * 更新 totalWindowsApplied / totalPointsProcessed 统计,
     * 并发射 windowApplied 信号。
     */
    QVector<double> applyWindow(const QVector<double> &data);

    // ── 查询接口 ──

    /** @brief 获取当前窗函数系数(最近一次generateWindow的结果) @return 窗系数向量 */
    QVector<double> windowCoefficients() const;

    /**
     * @brief 计算当前窗函数的相干增益
     * @return 相干增益值,定义为窗系数均值(sum/N)
     *
     * 相干增益表示窗函数对信号直流分量的衰减程度,
     * 矩形窗增益为1.0,其他窗增益小于1.0。
     */
    double coherentGain() const;

    /** @brief 获取累计统计信息快照 @return 统计结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /**
     * @brief 窗函数应用完成时发射
     * @param windowSize 本次加窗的窗长度
     */
    void windowApplied(int windowSize);

private:
    /**
     * @brief 计算修正贝塞尔函数I0(x),用于Kaiser窗
     * @param x 输入值(非负)
     * @return I0(x)近似值
     *
     * 使用级数展开: I0(x) = sum( ((x/2)^k / k!)^2 ), k=0,1,2,...
     * 收敛条件: 连续项的绝对值小于1e-12时截断。
     */
    static double besselI0(double x);

    WindowType     m_windowType;     ///< 当前窗函数类型
    int            m_windowSize;     ///< 当前窗长度(采样点数)
    QVector<double> m_coefficients;  ///< 当前窗系数缓存
    Stats          m_stats;          ///< 累计运行统计
};

#endif // DATAWINDOWMANAGER_H

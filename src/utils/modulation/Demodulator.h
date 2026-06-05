/**
 * @file Demodulator.h
 * @brief 解调器引擎 — AM/FM/PM/ASK/FSK/PSK解调
 *
 * 功能: 6种解调方法，从调制信号中恢复基带数据，
 *       支持自定义载波频率和符号率。
 *
 * 协作: ModbusMaster(协议解调) / FftEngine(频谱分析)
 */
#ifndef DEMODULATOR_H
#define DEMODULATOR_H

#include <QObject>
#include <QVector>
#include <QList>

class Demodulator : public QObject {
    Q_OBJECT

public:
    /** @brief 解调类型 */
    enum class ModType {
        AM,     ///< 幅度解调(包络检测)
        FM,     ///< 频率解调(鉴频)
        PM,     ///< 相位解调
        ASK,    ///< 幅移键控解调
        FSK,    ///< 频移键控解调
        PSK     ///< 相移键控解调
    };
    Q_ENUM(ModType)

    /** @brief 解调配置 */
    struct Config {
        double carrierFreq = 1000.0;    ///< 载波频率(Hz)
        double sampleRate = 10000.0;    ///< 采样率(Hz)
        double symbolRate = 100.0;      ///< 符号率(Baud)
        int    freqMark = 1200;         ///< FSK传号频率
        int    freqSpace = 800;         ///< FSK空号频率
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalSamplesDemod = 0;
        quint64 totalSymbolsDecoded = 0;
        quint64 totalBitErrors = 0;
        double  averageSnr = 0.0;
    };

    explicit Demodulator(QObject* parent = nullptr);

    void setModType(ModType type);
    void setConfig(const Config& config);

    /** @brief 解调连续数据 @param data I/Q或实信号 @return 解调输出 */
    QVector<double> demodulate(const QVector<double>& data);

    /** @brief 解调并解码数字符号 @param data 信号 @return 比特序列 */
    QVector<int> decodeSymbols(const QVector<double>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void demodulationComplete(int sampleCount);

private:
    QVector<double> demodAM(const QVector<double>& data) const;
    QVector<double> demodFM(const QVector<double>& data) const;
    QVector<double> demodPM(const QVector<double>& data) const;
    QVector<int> decodeASK(const QVector<double>& data) const;
    QVector<int> decodeFSK(const QVector<double>& data) const;
    QVector<int> decodePSK(const QVector<double>& data) const;

    ModType m_type;
    Config m_config;
    double m_snrSum;
    Stats m_stats;
};

#endif // DEMODULATOR_H

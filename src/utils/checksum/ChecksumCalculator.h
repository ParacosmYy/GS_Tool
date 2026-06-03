/**
 * @file ChecksumCalculator.h
 * @brief 校验和计算器
 * @author Serial Tool Team
 * @date 2026-06-02
 *
 * 支持 CRC8/16/32、异或、求和等多种校验算法。
 */

#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class ChecksumCalculator
 * @brief 多算法校验和计算引擎
 */
class ChecksumCalculator : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 校验和算法枚举
     */
    enum Algorithm {
        CRC8,           ///< CRC-8
        CRC16Ccitt,     ///< CRC-16 CCITT
        CRC16Modbus,    ///< CRC-16 Modbus
        CRC16Kermit,    ///< CRC-16 Kermit
        CRC32,          ///< CRC-32
        CRC32C,         ///< CRC-32C (Castagnoli)
        Xor8,           ///< 8位异或
        Sum8,           ///< 8位求和
        Sum16,          ///< 16位求和
        Sum32,          ///< 32位求和
        CustomCrc       ///< 自定义 CRC
    };
    Q_ENUM(Algorithm)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ChecksumCalculator(QObject *parent = nullptr);

    /**
     * @brief 计算校验和
     * @param data 输入数据
     * @param alg 算法类型
     * @return 校验和结果
     */
    quint64 calculate(const QByteArray &data, Algorithm alg) const;

    /**
     * @brief 使用自定义多项式计算 CRC
     * @param data 输入数据
     * @param polynomial CRC 多项式
     * @param width CRC 位宽（8/16/32）
     * @return CRC 结果
     */
    quint64 calculateCustom(const QByteArray &data, quint64 polynomial, int width) const;

    /**
     * @brief 获取算法名称
     * @param alg 算法枚举值
     * @return 算法名称字符串
     */
    static QString algorithmName(Algorithm alg);

    /**
     * @brief 获取算法的位宽（8/16/32）
     * @param alg 算法枚举值
     * @return 算法输出位宽
     */
    static int algorithmBitWidth(Algorithm alg);

    /**
     * @brief 获取算法的人类可读描述
     * @param alg 算法枚举值
     * @return 描述字符串，适合作为UI工具提示
     */
    static QString algorithmDescription(Algorithm alg);

    /**
     * @brief 使用所有内置算法计算同一份数据的校验和（对比视图）
     * @param data 输入数据
     * @return QMap<算法名, 校验和值>，不包含CustomCrc
     */
    QMap<QString, quint64> calculateAll(const QByteArray& data) const;

    // ---- 统计计数接口 ----
    /** @brief 获取累计计算次数 */
    quint64 totalCalculations() const;
    /** @brief 获取累计处理字节数 */
    quint64 totalBytesProcessed() const;
    /** @brief 重置所有校验和统计计数器 */
    void resetChecksumStatistics();

private:
    /** @brief 累计计算次数（calculate调用计数） */
    mutable quint64 m_totalCalculations = 0;
    /** @brief 累计处理字节数 */
    mutable quint64 m_totalBytesProcessed = 0;
};

#endif // CHECKSUMCALCULATOR_H

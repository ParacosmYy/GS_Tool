/**
 * @file DataTransformer.h
 * @brief 数据字节级转换管道，支持可配置的多步骤变换流水线
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 提供 ByteSwap/BitReverse/XorMask/Base64/Hex/EndianSwap 等变换步骤，
 * 可按顺序组合为管道应用于串口数据流，支持自定义变换函数扩展。
 */

#ifndef DATATRANSFORMER_H
#define DATATRANSFORMER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>
#include <functional>

/**
 * @class DataTransformer
 * @brief 数据字节级转换管道引擎
 *
 * 将多个 TransformStep 按顺序组合，对 QByteArray 依次执行变换。
 * 每次调用 transform() 会更新吞吐量统计并发射 transformed 信号。
 */
class DataTransformer : public QObject
{
    Q_OBJECT

public:
    /** @brief 变换类型枚举，定义所有支持的字节级变换操作 */
    enum class TransformType {
        ByteSwap,       ///< 字节组反转（N字节为一组倒序）
        BitReverse,     ///< 每字节的比特位反转
        XorMask,        ///< 异或掩码（每字节与 mask 异或）
        Base64Encode,   ///< Base64 编码
        Base64Decode,   ///< Base64 解码
        HexEncode,      ///< 十六进制编码
        HexDecode,      ///< 十六进制解码
        EndianSwap,     ///< 字节序交换（2/4/8字节宽度）
        NullTransform,  ///< 空变换（直通，不做任何修改）
        Custom          ///< 自定义变换函数
    };
    Q_ENUM(TransformType)

    /** @brief 单个变换步骤的描述 */
    struct TransformStep {
        TransformType type;   ///< 变换类型
        QVariant params;      ///< 算法特定参数（如 groupSize/mask/width）
        QString name;         ///< 步骤显示名称
    };

    /** @brief 累计运行统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;   ///< 累计变换调用次数
        quint64 totalBytesIn = 0;      ///< 累计输入字节数
        quint64 totalBytesOut = 0;     ///< 累计输出字节数
        quint64 transformErrors = 0;   ///< 累计变换错误次数
        double avgThroughput = 0.0;    ///< 平均吞吐量（bytes/ms）
    };

    /**
     * @brief 构造数据变换管道
     * @param parent 父对象
     */
    explicit DataTransformer(QObject *parent = nullptr);

    /**
     * @brief 按顺序对所有已注册步骤执行变换
     * @param input 输入字节数据
     * @return 变换后的字节数据；任何步骤出错时返回到出错前一步的结果
     */
    QByteArray transform(const QByteArray &input);

    /**
     * @brief 向管道末尾追加一个变换步骤
     * @param type  变换类型
     * @param params 算法参数（如 groupSize/mask/width）
     */
    void addStep(TransformType type, const QVariant &params = {});

    /**
     * @brief 移除指定索引处的步骤
     * @param index 步骤索引（0-based）
     */
    void removeStep(int index);

    /** @brief 清除所有变换步骤 */
    void clearSteps();

    /**
     * @brief 设置自定义变换函数（用于 TransformType::Custom 步骤）
     * @param fn 接收 QByteArray 并返回变换结果的函数对象
     */
    void setCustomTransform(const std::function<QByteArray(const QByteArray &)> &fn);

    /**
     * @brief 获取当前步骤列表的副本
     * @return 步骤列表
     */
    QList<TransformStep> steps() const;

    /**
     * @brief 获取累计统计信息
     * @return 统计结构体
     */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 变换完成时发射，携带最终输出 @param output 变换后的字节数据 */
    void transformed(const QByteArray &output);

    /** @brief 变换过程中发生错误时发射 @param message 错误描述（中文） */
    void error(const QString &message);

private:
    /**
     * @brief 对输入数据应用单个变换步骤
     * @param input 输入字节数据
     * @param step  变换步骤描述
     * @return 变换后的字节数据
     */
    QByteArray applyStep(const QByteArray &input, const TransformStep &step);

    QList<TransformStep> m_steps;                                    ///< 已注册的变换步骤列表
    std::function<QByteArray(const QByteArray &)> m_customFn;       ///< 自定义变换函数
    Stats m_stats;                                                   ///< 累计运行统计
};

#endif // DATATRANSFORMER_H

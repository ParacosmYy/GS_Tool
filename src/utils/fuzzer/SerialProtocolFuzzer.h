/**
 * @file SerialProtocolFuzzer.h
 * @brief 串口协议模糊测试器
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 面向串口通信协议的模糊测试引擎，支持六种变异策略：
 * 随机字节、位翻转、字节翻转、边界值、长度变异和字段级变异。
 * 使用确定性随机种子保证测试可重现，支持批量生成和CSV导出。
 */

#ifndef SERIALPROTOCOLFUZZER_H
#define SERIALPROTOCOLFUZZER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

/**
 * @class SerialProtocolFuzzer
 * @brief 串口协议模糊测试引擎
 *
 * 核心设计：
 * - 六种变异策略(RandomBytes/BitFlip/ByteFlip/BoundaryValues/LengthFuzz/FieldFuzz)
 * - 确定性种子保证同一配置下输出完全可重现
 * - 支持基于模板数据对指定字节范围进行定向变异
 * - 批量生成和迭代器式逐条获取两种工作模式
 * - CSV 导出完整的变异记录(迭代序号/变异偏移/描述)
 */
class SerialProtocolFuzzer : public QObject
{
    Q_OBJECT

public:
    /** @brief 模糊测试变异策略 */
    enum class FuzzStrategy {
        RandomBytes,      ///< 完全随机字节填充(可配置长度)
        BitFlip,          ///< 随机位翻转(可配置翻转密度)
        ByteFlip,         ///< 字节替换为边界值(0x00/0xFF/0x80等)
        BoundaryValues,   ///< 插入常见测试值(0/MAX/MIN/off-by-one)
        LengthFuzz,       ///< 长度变异(0/1/MAX-1/MAX/MAX+1)
        FieldFuzz         ///< 仅对模板数据指定偏移范围进行变异
    };
    Q_ENUM(FuzzStrategy)

    /** @brief 模糊测试配置 */
    struct FuzzConfig {
        QByteArray templateData;    ///< 模板数据(FieldFuzz/LengthFuzz 策略的基础)
        QList<int> fuzzFields;      ///< 指定变异的字节偏移列表(FieldFuzz 使用)
        FuzzStrategy strategy = FuzzStrategy::RandomBytes; ///< 变异策略
        int iterationCount = 100;   ///< 计划生成的总迭代次数
        int maxMutationRate = 10;   ///< 每次迭代最大变异点数(BitFlip/ByteFlip)
        quint32 randomSeed = 0xDEADBEEFu; ///< 随机种子(0=使用系统时间)
    };

    /** @brief 单次模糊测试结果 */
    struct FuzzResult {
        int iteration = 0;              ///< 迭代序号(从1开始)
        QByteArray fuzzedData;          ///< 变异后的数据
        QString description;            ///< 人类可读的变异描述
        QList<int> mutatedOffsets;      ///< 本次变异涉及的偏移量列表
    };

    /** @brief 累计统计数据 */
    struct Stats {
        quint64 totalIterations = 0;      ///< 累计迭代次数
        quint64 totalBytesGenerated = 0;  ///< 累计生成的字节总数
        quint64 totalMutations = 0;       ///< 累计变异操作次数
        quint64 totalFuzzFields = 0;      ///< 累计变异的字段/偏移点数
        quint64 avgMutationRate = 0;      ///< 平均每次迭代变异点数
        quint64 crashesDetected = 0;      ///< 检测到的异常/崩溃次数(外部标记)
    };

    /** @brief 构造串口协议模糊测试器 @param parent 父对象 */
    explicit SerialProtocolFuzzer(QObject *parent = nullptr);

    // ── 配置接口 ──

    /** @brief 设置模糊测试配置(重置后生效)
     *  @param config 配置结构体
     */
    void setConfig(const FuzzConfig &config);

    /** @brief 获取当前配置 @return 配置结构体只读引用 */
    const FuzzConfig &config() const;

    // ── 数据生成 ──

    /** @brief 生成单条模糊测试数据
     *  @return 变异结果，配置无效时返回空结果
     */
    FuzzResult generate();

    /** @brief 批量生成指定数量的模糊测试数据
     *  @param count 生成条数(0=使用配置中的 iterationCount)
     *  @return 结果列表
     */
    QList<FuzzResult> generateBatch(int count = 0);

    /** @brief 迭代器式获取下一条结果
     *
     *  基于 m_currentIndex 递进，到达 iterationCount 后 hasMore() 返回 false。
     *
     *  @return 变异结果
     */
    FuzzResult next();

    /** @brief 获取当前迭代位置(从0开始) @return 当前迭代索引 */
    int currentIteration() const;

    /** @brief 是否还有未生成的迭代 @return true=仍有剩余迭代 */
    bool hasMore() const;

    // ── 导出 ──

    /** @brief 将结果列表导出为CSV格式
     *  @param results 结果列表
     *  @return CSV格式字符串(列: Iteration,Mutations,Offsets,HexData,Description)
     */
    QString exportResults(const QList<FuzzResult> &results) const;

    // ── 统计 ──

    /** @brief 获取累计统计数据快照 @return Stats 结构体副本 */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器和迭代位置 */
    void resetStatistics();

signals:
    /** @brief 单条模糊数据生成完成 @param result 变异结果 */
    void fuzzGenerated(const SerialProtocolFuzzer::FuzzResult &result);

    /** @brief 批量生成完成 @param count 本批次生成的总条数 */
    void batchComplete(int count);

private:
    // ── 各策略的变异实现 ──

    /** @brief 策略: 生成完全随机的字节序列 */
    FuzzResult doRandomBytes();

    /** @brief 策略: 对模板数据进行随机位翻转 */
    FuzzResult doBitFlip();

    /** @brief 策略: 将随机字节替换为边界值 */
    FuzzResult doByteFlip();

    /** @brief 策略: 用常见边界测试值填充整个数据 */
    FuzzResult doBoundaryValues();

    /** @brief 策略: 变异数据长度(截断/扩展/空/单字节) */
    FuzzResult doLengthFuzz();

    /** @brief 策略: 仅对指定偏移范围的字段进行变异 */
    FuzzResult doFieldFuzz();

    /** @brief 生成一个 [0, max) 范围内的随机数 @param max 上界(不含) */
    quint32 randomUInt(quint32 max);

    /** @brief 获取一个随机字节 */
    quint8 randomByte();

    /** @brief 获取一个常见边界测试值 */
    quint8 boundaryValue();

    // ── 成员 ──

    FuzzConfig m_config;          ///< 当前配置
    int m_currentIndex = 0;       ///< 当前迭代索引
    quint32 m_rngState = 0;       ///< PRNG 内部状态(Xorshift32)

    static constexpr int kMaxDataSize = 4096; ///< 单次生成的数据最大字节数

    // ── 统计计数器 ──
    quint64 m_totalIterations = 0;
    quint64 m_totalBytesGenerated = 0;
    quint64 m_totalMutations = 0;
    quint64 m_totalFuzzFields = 0;
    quint64 m_crashesDetected = 0;
};

#endif // SERIALPROTOCOLFUZZER_H

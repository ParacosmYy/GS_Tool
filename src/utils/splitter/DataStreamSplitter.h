/**
 * @file DataStreamSplitter.h
 * @brief 数据流分割器 -- 将连续串口数据流按可配置规则拆分为独立帧/数据包
 *
 * 支持4种分割模式: 分隔符匹配、固定长度、报头长度字段、超时切割。
 * 通过内部缓冲区累积部分数据，在完整帧就绪时输出。
 * 协作: SerialConnection(串口数据源) / DataLogger(帧记录) / ProtocolEngine(协议解析)
 */
#ifndef DATASTREAMSPLITTER_H
#define DATASTREAMSPLITTER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
/**
 * @brief 数据流分割器
 *
 * 将连续的串口/网络字节流按可配置规则拆分为离散帧。
 * 支持 Delimiter / FixedLength / HeaderLength / Timeout 四种模式，
 * 可在运行时动态切换规则。分割完成后通过信号通知调用方。
 */
class DataStreamSplitter : public QObject {
    Q_OBJECT

public:
    /** @brief 分割模式 */
    enum class SplitMode {
        Delimiter,      ///< 分隔符模式: 遇到指定字节序列时切割
        FixedLength,    ///< 固定长度模式: 每N字节切割一次
        HeaderLength,   ///< 报头长度模式: 从报头指定位置读取长度字段
        Timeout         ///< 超时模式: 收到数据后N毫秒无新数据则切割
    };
    Q_ENUM(SplitMode)

    /** @brief 分割规则配置 */
    struct SplitRule {
        SplitMode mode = SplitMode::Delimiter;  ///< 分割模式
        QByteArray delimiter;                   ///< Delimiter模式: 分隔符字节序列
        int fixedLength = 0;                    ///< FixedLength模式: 固定帧长度(字节)
        int headerLenPos = 0;                   ///< HeaderLength模式: 长度字段在帧中的起始偏移
        int headerLenOffset = 0;                ///< HeaderLength模式: 长度字段值偏移量(长度字段值+偏移=总帧长)
        int headerLenSize = 1;                  ///< HeaderLength模式: 长度字段字节数(1/2/4)
        int timeoutMs = 100;                    ///< Timeout模式: 超时毫秒数
    };

    /** @brief 分割结果帧 */
    struct SplitResult {
        int frameIndex = 0;                     ///< 帧序号(从0开始递增)
        QByteArray data;                        ///< 帧数据(不含分隔符)
        qint64 timestamp = 0;                   ///< 帧完成时刻的时间戳(epoch毫秒)
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalFramesSplit = 0;           ///< 已分割出的总帧数
        quint64 totalBytesProcessed = 0;        ///< 已处理的总字节数
        quint64 totalDiscarded = 0;             ///< 因错误丢弃的总字节数
        quint64 currentRuleIndex = 0;           ///< 当前规则索引(预留多规则)
        quint64 splitErrors = 0;                ///< 分割错误次数
        quint64 avgFrameSize = 0;               ///< 平均帧大小(字节)
    };

    /** @brief 构造数据流分割器 @param parent 父对象 */
    explicit DataStreamSplitter(QObject* parent = nullptr);
    /** @brief 析构时停止超时定时器 */
    ~DataStreamSplitter() override;

    // ---- 规则配置 ----

    /**
     * @brief 设置分割规则
     *
     * 切换规则时会清空内部缓冲区并重置帧计数器。
     * @param rule 分割规则配置
     */
    void setRule(const SplitRule& rule);

    /** @brief 获取当前分割规则 @return 当前规则的const引用 */
    const SplitRule& rule() const;

    // ---- 数据处理 ----

    /**
     * @brief 输入原始字节流，返回所有完整帧
     *
     * 数据追加到内部缓冲区，根据当前模式提取完整帧。
     * Timeout模式下同时启动/重启超时定时器。
     * @param data 原始输入字节
     * @return 本轮提取到的完整帧列表(可能为空)
     */
    QList<SplitResult> process(const QByteArray& data);

    /**
     * @brief 清空内部缓冲区并重置帧计数器
     *
     * 不影响统计信息和当前规则配置。
     */
    void reset();

    /**
     * @brief 获取当前缓冲区中的未分割数据
     * @return 缓冲区内容的副本
     */
    QByteArray currentBuffer() const;

    // ---- 导出 ----

    /**
     * @brief 将帧列表导出为CSV格式
     *
     * CSV列: 帧序号, 时间戳, 帧长度, Hex数据
     * @param frames 待导出的帧列表
     * @param filePath 目标文件路径
     * @return 成功返回true
     */
    bool exportFrames(const QList<SplitResult>& frames, const QString& filePath) const;

    // ---- 统计 ----

    /** @brief 获取运行时统计信息 @return 统计数据的const引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器(不影响规则和缓冲区) */
    void resetStatistics();

signals:
    /**
     * @brief 新帧可用信号
     * @param frame 分割出的帧数据
     */
    void frameAvailable(const DataStreamSplitter::SplitResult& frame);

    /**
     * @brief 分割错误信号
     * @param errorMessage 错误描述
     */
    void splitterError(const QString& errorMessage);

private:
    /**
     * @brief 分隔符模式分割
     * 在缓冲区中搜索分隔符，提取所有完整帧。
     * @param results 输出: 提取到的帧列表
     */
    void splitByDelimiter(QList<SplitResult>& results);

    /**
     * @brief 固定长度模式分割
     * 缓冲区达到fixedLength时切割一帧。
     * @param results 输出: 提取到的帧列表
     */
    void splitByFixedLength(QList<SplitResult>& results);

    /**
     * @brief 报头长度模式分割
     * 从缓冲区headerLenPos处读取headerLenSize字节的长度值，
     * 加上headerLenOffset得到总帧长，缓冲区达到总长时切割。
     * @param results 输出: 提取到的帧列表
     */
    void splitByHeaderLength(QList<SplitResult>& results);

    /**
     * @brief 超时模式分割回调
     * 超时定时器触发时将缓冲区全部内容作为一帧输出。
     */
    void onTimeout();

    /**
     * @brief 从缓冲区指定位置读取长度字段
     * 支持1/2/4字节，大端序。
     * @param pos 起始偏移
     * @param size 字段字节数
     * @return 读取到的长度值，失败返回-1
     */
    int readLengthField(int pos, int size) const;

    /**
     * @brief 更新平均帧大小统计
     * @param frameSize 新帧的字节大小
     */
    void updateAvgFrameSize(quint64 frameSize);

    /**
     * @brief 为结果帧填充元数据并递增帧计数
     * @param data 帧数据
     * @return 完整的SplitResult
     */
    SplitResult makeResult(const QByteArray& data);

    SplitRule m_rule;                       ///< 当前分割规则
    QByteArray m_buffer;                    ///< 内部累积缓冲区
    int m_frameIndex;                       ///< 帧序号计数器
    Stats m_stats;                          ///< 运行时统计
    QTimer* m_timeoutTimer;                 ///< Timeout模式的定时器
    QElapsedTimer m_lastDataTime;           ///< 上次收到数据的时间(用于超时检测)
};
#endif // DATASTREAMSPLITTER_H


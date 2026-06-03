/**
 * @file FrameParser.h
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 状态流转: Idle -> HeaderMatching -> LengthReceiving -> PayloadReceiving
 *           -> ChecksumVerifying -> FooterMatching -> Done
 *
 * 安全机制:
 *   1. 帧长度上限检查 (maxFrameLength) - 默认1024字节，超长帧自动丢弃
 *   2. 状态机超时机制 - 帧头匹配后超时自动重置
 *   3. 硬性上限 kMaxFrameSize (4096) - 无论配置如何，单帧不超过此值
 *   4. 帧计数器 - 累计解析帧数/错误帧数，用于诊断和统计
 *   5. 超时恢复 - 超时后清除所有中间缓冲区和匹配进度
 *
 * 设计模式: 状态模式(State) - 解析行为由当前状态驱动，状态转换由接收到的字节决定
 */

#ifndef FRAMEPARSER_H
#define FRAMEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>
#include <QTimer>
#include "protocol/parser/FrameDefinition.h"
#include "utils/crypto/CRC.h"

/**
 * @brief 帧解析状态机 - 从字节流中实时解析协议帧
 *
 * 协作关系:
 *   - ProtocolBridgeManager: 持有 FrameParser，将串口数据路由到 feed()
 *   - FrameDefinition: 定义帧结构（帧头/帧尾/校验/字段布局）
 *   - ChartModel / ProtocolView: 下游消费者，连接 frameParsed/frameError 信号
 */
class FrameParser : public QObject {
    Q_OBJECT

public:
    /** @brief 构造帧解析器，默认帧上限1024字节 */
    explicit FrameParser(QObject* parent = nullptr);
    /** @brief 析构函数，释放超时定时器等资源 */
    ~FrameParser() override;

    /** @brief 设置帧格式定义，设置后自动 reset() */
    void setDefinition(const FrameDefinition& def);

    /** @brief 获取当前帧格式定义 */
    FrameDefinition definition() const;

    /** @brief 喂入字节流数据，逐字节送入状态机处理。空数据直接忽略 */
    void feed(const QByteArray& data);

    /** @brief 重置解析器状态（清空缓冲区，回到Idle），不清零计数器 */
    void reset();

    /** @brief 获取已解析的帧计数 */
    quint64 frameCount() const;

    /** @brief 获取错误帧计数 */
    quint64 errorCount() const;

    /**
     * @brief 设置帧长度上限，传入值 <= 0 或 > kMaxFrameSize 时使用 kMaxFrameSize
     * 默认值为 kDefaultMaxFrameLength (1024字节)
     */
    void setMaxFrameLength(int maxLen);

    /** @brief 获取当前帧长度上限 */
    int maxFrameLength() const;

    /** @brief 设置状态机超时阈值（毫秒），0 禁用 */
    void setFrameTimeout(int timeoutMs);

    /** @brief 获取当前超时阈值（毫秒），0 表示禁用 */
    int frameTimeout() const;

    /* —— 统计计数器接口 —— */

    /** @brief 获取成功解析的帧总数 */
    quint64 totalFramesParsed() const;

    /** @brief 获取累计输入的字节总数 */
    quint64 totalBytesInput() const;

    /** @brief 获取校验和错误次数(CRC/Sum/异或不匹配) */
    quint64 totalChecksumErrors() const;

    /** @brief 获取累计溢出次数（帧超过最大长度被丢弃） */
    quint64 totalOverflows() const;

    /** @brief 获取累计解析错误次数(含格式错/长度错/帧尾不匹配/超时) */
    quint64 totalParseErrors() const;

    /** @brief 获取成功解析帧中的有效数据字节总数 */
    quint64 totalBytesParsed() const;

    /** @brief 重置所有统计计数器（帧数/字节/校验错误/溢出/解析错误/已解析字节） */
    void resetStats();

signals:
    /** @brief 帧解析成功，fields 为各字段名->值映射 */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief 帧解析错误 */
    void frameError(const QString& reason, const QByteArray& rawFrame);

private:
    /** @brief 解析状态枚举 */
    enum class State {
        Idle,               ///< 等待帧头
        HeaderMatching,     ///< 正在匹配帧头
        LengthReceiving,    ///< 正在接收长度字段
        PayloadReceiving,   ///< 正在接收有效数据
        ChecksumVerifying,  ///< 正在接收校验字段
        FooterMatching      ///< 正在匹配帧尾
    };

    // ---- 状态处理方法 ----

    /** @brief 逐字节状态机处理入口 @param byte 输入字节 */
    void processByte(unsigned char byte);
    /** @brief 帧头匹配状态处理 @param byte 输入字节 */
    void handleHeaderMatching(unsigned char byte);
    /** @brief 长度字段接收状态处理 @param byte 输入字节 */
    void handleLengthReceiving(unsigned char byte);
    /** @brief 有效载荷接收状态处理 @param byte 输入字节 */
    void handlePayloadReceiving(unsigned char byte);
    /** @brief 长度字段模式下帧完成处理(校验→帧尾→完成) */
    void processCompletePayload();
    /** @brief CRC校验验证，通过返回true */
    bool handleCrcValidation();
    /** @brief 校验字段接收状态处理 @param byte 输入字节 */
    void handleChecksumVerifying(unsigned char byte);
    /** @brief 帧尾匹配状态处理 @param byte 输入字节 */
    void handleFooterMatching(unsigned char byte);

    // ---- 辅助方法（实现在 FrameParserHelpers.cpp） ----

    /** @brief 从完整帧数据中按字段定义提取各字段值 @param frameData 完整帧数据 @return 字段名->值映射 */
    QVariantMap extractFields(const QByteArray& frameData) const;
    /** @brief 验证帧数据的校验和 @param frameData 完整帧数据 @return 校验通过返回true */
    bool verifyChecksum(const QByteArray& frameData) const;
    /** @brief 从帧数据中解析长度字段值 @param frameData 完整帧数据 @return 长度字段值 */
    int parseLengthField(const QByteArray& frameData) const;
    /** @brief 计算给定数据的校验和 @param data 待计算数据 @return 校验和字节数组 */
    QByteArray computeChecksum(const QByteArray& data) const;

    // ---- 状态机基础设施 ----

    /** @brief 检查帧接收是否超时 @return 超时返回true */
    bool checkTimeout();
    /** @brief 完成一帧解析，提取字段并发射frameParsed信号 */
    void completeFrame();
    /** @brief 停止超时检查定时器 */
    void stopTimeoutTimer();
    /** @brief 启动超时检查定时器 */
    void startTimeoutTimer();
    /** @brief 重置中间状态(缓冲区/匹配进度/期望长度)，回到Idle */
    void resetIntermediateState();

    // ---- 成员变量 ----
    State m_state = State::Idle;            ///< 当前解析状态
    FrameDefinition m_def;                  ///< 当前帧格式定义
    QByteArray m_buffer;                    ///< 当前帧缓冲区
    int m_headerMatchPos = 0;               ///< 帧头匹配进度
    int m_footerMatchPos = 0;               ///< 帧尾匹配进度
    int m_expectedPayload = 0;              ///< 期望的有效数据长度
    quint64 m_frameCount = 0;               ///< 成功解析帧计数(兼容旧接口)
    quint64 m_errorCount = 0;               ///< 错误帧计数(兼容旧接口)

    quint64 m_totalFramesParsed = 0;        ///< 成功解析帧总数
    quint64 m_totalBytesInput = 0;          ///< 累计输入字节总数
    quint64 m_totalChecksumErrors = 0;      ///< 校验和错误总数
    quint64 m_totalOverflows = 0;           ///< 累计溢出次数
    quint64 m_totalParseErrors = 0;         ///< 累计解析错误次数(含格式/长度/帧尾/超时)
    quint64 m_totalBytesParsed = 0;         ///< 成功解析帧中的有效数据字节总数

    int m_maxFrameLength = kDefaultMaxFrameLength; ///< 帧长度上限（默认1024字节）
    int m_frameTimeoutMs = 500;             ///< 帧超时阈值（毫秒），0=禁用
    QElapsedTimer m_frameTimer;             ///< 帧接收计时器
    QTimer* m_timeoutCheckTimer = nullptr;  ///< 独立的超时检查定时器

    static constexpr int kMaxFrameSize = 4096;          ///< 硬性上限
    static constexpr int kDefaultMaxFrameLength = 1024;  ///< 默认帧长度上限
};

#endif // FRAMEPARSER_H

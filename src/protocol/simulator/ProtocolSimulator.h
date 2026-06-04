/**
 * @file ProtocolSimulator.h
 * @brief 协议响应模拟器 -- 自动应答测试工具
 *
 * 用户定义请求-响应规则对，模拟器自动匹配输入数据并生成响应。
 * 支持精确匹配与正则匹配，可配置响应延迟模拟真实设备时序。
 * 默认应答兜底，JSON脚本导入导出，适合在无硬件环境下测试协议处理逻辑。
 *
 * 协作: SerialConnection(数据源)/ProtocolEngine(协议层)/自动化测试框架
 */

#ifndef PROTOCOLSIMULATOR_H
#define PROTOCOLSIMULATOR_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>

/**
 * @brief 协议响应模拟器
 *
 * 核心流程:
 *   1. 用户通过 addRule() 定义请求-响应规则
 *   2. 调用 start() 启动模拟器
 *   3. 通过 processData() 喂入收到的数据
 *   4. 模拟器匹配规则后按配置延迟发射响应
 *   5. 统计模块记录匹配/未匹配/延迟等指标
 */
class ProtocolSimulator : public QObject {
    Q_OBJECT

public:
    /** @brief 响应规则定义 */
    struct ResponseRule {
        int id = -1;                       ///< 规则唯一ID(自动分配)
        QString name;                      ///< 规则名称(用户可读)
        QByteArray requestPattern;         ///< 请求匹配模式(精确匹配内容或正则表达式)
        QByteArray responseData;           ///< 匹配成功后的响应数据
        int delayMs = 0;                   ///< 模拟响应延迟(ms, 0=立即)
        bool useRegex = false;             ///< 是否使用正则匹配(false=精确匹配)
        bool enabled = true;               ///< 规则是否启用
        int matchCount = 0;                ///< 该规则累计匹配次数
    };

    /** @brief 运行统计 */
    struct SimulationStats {
        quint64 totalRequestsReceived = 0;  ///< 累计接收请求数
        quint64 totalResponsesSent = 0;     ///< 累计发送响应数
        quint64 totalMatches = 0;           ///< 累计匹配成功次数
        quint64 totalMisses = 0;            ///< 累计未匹配次数
        quint64 totalDelayedResponses = 0;  ///< 累计延迟响应次数
        quint64 totalBytesReceived = 0;     ///< 累计接收字节数
        quint64 totalBytesSent = 0;         ///< 累计发送字节数
        double avgResponseTimeMs = 0.0;     ///< 平均响应时间(ms)
        int activeRules = 0;                ///< 当前活跃规则数(已启用)
        int peakPendingRequests = 0;        ///< 峰值待处理请求数
    };

    //-- 构造/析构 --//
    explicit ProtocolSimulator(QObject* parent = nullptr); ///< 构造(初始化定时器和统计)
    ~ProtocolSimulator() override;                         ///< 析构(停止定时器)

    //-- 规则管理 --//
    int addRule(const ResponseRule& rule);                 ///< 添加响应规则(返回分配的ID)
    bool removeRule(int id);                               ///< 移除规则(按ID)
    bool updateRule(int id, const ResponseRule& rule);     ///< 更新规则(按ID)
    void enableRule(int id, bool enabled);                 ///< 启用/禁用规则(按ID)
    QList<ResponseRule> rules() const;                     ///< 获取所有规则列表

    //-- 数据处理 --//
    QByteArray processData(const QByteArray& incoming);    ///< 处理输入数据(匹配规则并返回响应)

    //-- 模拟控制 --//
    void start();                                          ///< 启动模拟器(开始接收和处理数据)
    void stop();                                           ///< 停止模拟器(暂停处理,保留规则)
    bool isRunning() const;                                ///< 模拟器是否正在运行

    //-- 配置 --//
    void setDefaultResponse(const QByteArray& data);       ///< 设置默认响应(未匹配任何规则时的兜底)

    //-- 脚本持久化 --//
    bool loadScript(const QString& filePath);              ///< 从JSON文件加载规则脚本
    bool saveScript(const QString& filePath) const;        ///< 保存规则脚本到JSON文件

    //-- 统计接口 --//
    const SimulationStats& stats() const;                  ///< 获取运行统计
    void resetStatistics();                                ///< 重置所有统计计数器

signals:
    /** @brief 接收到请求数据 @param data 接收到的原始数据 */
    void requestReceived(const QByteArray& data);
    /** @brief 发送响应数据 @param data 响应数据 @param matchedRuleId 匹配的规则ID(-1=默认响应) */
    void responseSent(const QByteArray& data, int matchedRuleId);
    /** @brief 规则匹配成功 @param ruleId 匹配的规则ID */
    void ruleMatched(int ruleId);
    /** @brief 未找到匹配规则 @param data 未匹配的原始数据 */
    void noMatchFound(const QByteArray& data);
    /** @brief 模拟器运行状态变化 @param running true=运行中 false=已停止 */
    void simulatorStateChanged(bool running);

private:
    /** @brief 待发送的延迟响应 */
    struct PendingResponse {
        QByteArray data;    ///< 响应数据
        qint64 sendAtMs;    ///< 计划发送时间(epoch ms)
        int ruleId;         ///< 关联的规则ID
    };

    //-- 私有方法 --//
    int findMatchingRule(const QByteArray& data) const;    ///< 查找匹配的规则(返回规则索引, -1=未找到)
    void processPendingResponses();                        ///< 检查并发送到期的延迟响应
    void updateActiveRuleCount();                          ///< 更新活跃规则计数统计

    //-- 成员变量 --//
    QList<ResponseRule> m_rules;             ///< 响应规则列表
    int m_nextRuleId = 1;                    ///< 下一个规则ID(自增分配)
    bool m_running = false;                  ///< 模拟器运行状态
    QByteArray m_defaultResponse;            ///< 默认兜底响应(空=不发送)
    QTimer* m_delayTimer = nullptr;          ///< 延迟响应检查定时器
    QVector<PendingResponse> m_pendingResponses; ///< 待发送的延迟响应队列
    SimulationStats m_stats;                 ///< 运行统计

    //-- 累计统计辅助 --//
    quint64 m_sumResponseTimeMs = 0;         ///< 响应时间总和(用于计算平均值)
    quint64 m_responseTimeCount = 0;         ///< 响应时间采样计数

    static constexpr int kDelayCheckIntervalMs = 10; ///< 延迟响应检查间隔(ms)
};

#endif // PROTOCOLSIMULATOR_H

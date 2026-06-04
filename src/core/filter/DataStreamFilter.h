/** @file DataStreamFilter.h @brief 数据流过滤器引擎 -- 可配置过滤链/实时数据筛选 */
#ifndef DATASTREAMFILTER_H
#define DATASTREAMFILTER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QRegularExpression>

/**
 * @brief 数据流过滤器引擎
 *
 * 对串口/网络等数据流应用可配置过滤链。支持5种规则类型和4种操作，
 * 规则按添加顺序依次执行，实现实时数据筛选。
 * 协作: Pipeline(数据管道) / TerminalWidget(终端显示) / DataLogger(数据记录)
 */
class DataStreamFilter : public QObject {
    Q_OBJECT

public:
    /** @brief 过滤操作类型 */
    enum class FilterOp {
        Pass,       ///< 通过(默认，不修改数据)
        Drop,       ///< 丢弃整条数据包
        Modify,     ///< 替换/修改数据内容
        Alert       ///< 告警(通过数据但发出信号)
    };
    Q_ENUM(FilterOp)

    /** @brief 规则匹配类型 */
    enum class FilterType {
        ByteRange,      ///< 字节范围匹配(Hex模式: "0x1A 0x2B")
        Regex,          ///< 正则表达式匹配(文本模式)
        Threshold,      ///< 数值阈值匹配(解释为数值比较)
        LengthRange,    ///< 数据长度范围匹配
        CustomPattern   ///< 自定义Hex模式匹配("AA BB CC ?? DD")
    };
    Q_ENUM(FilterType)

    /** @brief 过滤规则配置 */
    struct FilterRule {
        int id = 0;                          ///< 规则唯一ID(引擎自动分配)
        FilterType type = FilterType::Regex; ///< 规则类型
        QString name;                        ///< 规则名称(用户可读)
        QString pattern;                     ///< 正则表达式或Hex模式字符串
        double minThreshold = 0.0;           ///< Threshold类型: 最小阈值
        double maxThreshold = 0.0;           ///< Threshold类型: 最大阈值
        int minLength = 0;                   ///< LengthRange类型: 最小长度
        int maxLength = 0;                   ///< LengthRange类型: 最大长度
        QByteArray replacePattern;           ///< Modify操作: 替换后的数据(Hex编码)
        FilterOp op = FilterOp::Pass;        ///< 匹配时执行的操作
        bool enabled = true;                 ///< 规则是否启用
        bool caseSensitive = false;          ///< 正则是否区分大小写
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalInputPackets = 0;       ///< 输入数据包总数
        quint64 totalPassed = 0;             ///< 通过的数据包数
        quint64 totalDropped = 0;            ///< 丢弃的数据包数
        quint64 totalModified = 0;           ///< 修改的数据包数
        quint64 totalAlerts = 0;             ///< 告警触发次数
        quint64 totalRuleEvaluations = 0;    ///< 规则评估总次数
        quint64 totalRegexMatches = 0;       ///< 正则匹配成功次数
        quint64 totalThresholdChecks = 0;    ///< 阈值检查次数
        int activeRules = 0;                 ///< 当前启用规则数
        int peakQueueSize = 0;               ///< 历史批处理队列峰值
    };

    /** @brief 构造数据流过滤器 @param parent 父对象 */
    explicit DataStreamFilter(QObject* parent = nullptr);

    // ---- 规则管理 ----
    /** @brief 添加过滤规则(自动分配ID) @param rule 规则配置(忽略rule.id) @return 分配的规则ID */
    int addRule(const FilterRule& rule);
    /** @brief 移除指定ID的规则 @param id 规则ID @return 成功返回true */
    bool removeRule(int id);
    /** @brief 更新指定ID的规则 @param id 规则ID @param rule 新规则配置 @return 成功返回true */
    bool updateRule(int id, const FilterRule& rule);
    /** @brief 启用/禁用指定规则 @param id 规则ID @param enabled 启用状态 */
    void enableRule(int id, bool enabled);
    /** @brief 获取所有规则列表 @return 规则列表 */
    QList<FilterRule> rules() const;
    /** @brief 获取指定ID的规则 @param id 规则ID @return 规则配置，未找到时id=0 */
    FilterRule rule(int id) const;
    /** @brief 清空所有规则 */
    void clearRules();

    // ---- 数据处理 ----
    /** @brief 处理单条数据(依次应用所有启用的规则) @param input 输入数据 @return 处理后的数据(Drop时返回空) */
    QByteArray process(const QByteArray& input);
    /** @brief 批量处理数据 @param inputs 输入数据列表 @return 处理后的数据列表(已过滤Drop的数据) */
    QList<QByteArray> processBatch(const QList<QByteArray>& inputs);

    // ---- 持久化 ----
    /** @brief 导出规则到JSON文件 @param filePath 文件路径 @return 成功返回true */
    bool exportRules(const QString& filePath);
    /** @brief 从JSON文件导入规则 @param filePath 文件路径 @return 成功返回true */
    bool importRules(const QString& filePath);

    // ---- 统计 ----
    /** @brief 获取运行时统计信息 @return 统计数据的const引用 */
    const Stats& stats() const;
    /** @brief 重置所有统计计数器(不影响规则列表) */
    void resetStatistics();

signals:
    /** @brief 数据包通过(未被任何规则Drop) @param data 通过的数据 */
    void packetPassed(const QByteArray& data);
    /** @brief 数据包被丢弃 @param data 被丢弃的原始数据 */
    void packetDropped(const QByteArray& data);
    /** @brief 数据包被修改 @param before 修改前的数据 @param after 修改后的数据 */
    void packetModified(const QByteArray& before, const QByteArray& after);
    /** @brief 告警触发 @param ruleId 触发的规则ID @param data 触发告警的数据 */
    void alertTriggered(int ruleId, const QByteArray& data);

private:
    /** @brief 检查数据是否匹配指定规则 @param rule 过滤规则 @param data 待检查数据 @return 匹配返回true */
    bool matchesRule(const FilterRule& rule, const QByteArray& data);
    /** @brief 应用规则操作到数据 @param rule 过滤规则 @param data 输入数据 @return 操作后的数据 */
    QByteArray applyRule(const FilterRule& rule, const QByteArray& data);

    /** @brief 将Hex字符串转换为字节数组 @param hexStr Hex字符串(空格分隔) @return 字节数组 */
    QByteArray hexToBytes(const QString& hexStr) const;
    /** @brief 将字节数据解释为数值 @param data 二进制数据 @return 解释后的数值 */
    double dataToValue(const QByteArray& data) const;
    /** @brief 匹配自定义Hex模式(支持??通配符) @param pattern Hex模式 @param data 待匹配数据 @return 匹配返回true */
    bool matchHexPattern(const QByteArray& pattern, const QByteArray& data) const;

    QList<FilterRule> m_rules;          ///< 规则列表(按添加顺序)
    int m_nextRuleId;                   ///< 下一个分配的规则ID
    Stats m_stats;                      ///< 运行时统计

    QRegularExpression m_cachedRegex;   ///< 缓存的已编译正则
    int m_cachedRegexRuleId;            ///< 缓存正则对应的规则ID(-1=无效)
};

#endif // DATASTREAMFILTER_H

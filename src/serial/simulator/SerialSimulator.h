/**
 * @file SerialSimulator.h
 * @brief 串口设备模拟器 -- 无需真实硬件的测试替身
 *
 * 模拟串口设备的命令-响应交互行为，支持:
 *   - 响应规则: 输入模式匹配 → 生成输出(固定文本/计算值/随机数据)
 *   - 延迟模拟: 固定延迟 + 随机抖动，还原真实设备响应时序
 *   - 错误注入: 位错误/丢字节/重复字节/帧错误，测试上层容错
 *   - 设备配置: GPS/Modbus从站/传感器阵列/自定义，一键切换预设行为
 *   - 脚本模式: JSON脚本加载复杂交互序列
 *   - 回显模式: 原样回传接收数据
 *
 * 协作关系:
 *   - 上层调用 feed(input) → SerialSimulator → 返回 response QByteArray
 *   - 可替代真实串口连接用于UI/逻辑测试
 */
#ifndef SERIALSIMULATOR_H
#define SERIALSIMULATOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief 响应规则 -- 输入模式到输出的映射
 */
struct ResponseRule {
    QByteArray pattern;         ///< 输入匹配模式(精确匹配或通配符 *)
    QByteArray response;        ///< 响应内容模板(支持 {random} {counter} {hex} 占位符)
    bool isWildcard = false;    ///< 是否使用通配符匹配
    int matchLength = 0;        ///< 精确匹配时的模式长度(缓存)
};

/**
 * @brief 模拟器配置类型 -- 预定义设备行为模板
 */
enum class SimulatorProfile : int {
    Custom = 0,         ///< 自定义规则
    Gps,                ///< GPS NMEA 模拟器
    ModbusSlave,        ///< Modbus RTU 从站
    SensorArray,        ///< 多通道传感器阵列
    Echo,               ///< 回显模式
    ProfileCount        ///< 配置类型计数(用于数组大小)
};

/**
 * @brief 运行统计数据 -- 模拟器工作期间的累计统计
 */
struct SimulatorStats {
    quint64 totalRequestsProcessed = 0;     ///< 累计处理的请求总数
    quint64 totalResponsesGenerated = 0;    ///< 累计生成的响应总数
    quint64 totalErrorsInjected = 0;        ///< 累计注入的错误总数
    quint64 totalBytesDropped = 0;          ///< 累计丢弃的字节总数
    double avgResponseDelayMs = 0.0;        ///< 平均响应延迟(ms)
    quint64 requestsByProfile[5] = {};      ///< 按设备配置统计的请求数(索引=SimulatorProfile枚举值)
};

/**
 * @brief 串口设备模拟器 -- 模拟真实设备的命令-响应行为
 *
 * 使用方式:
 *   1. loadProfile(SimulatorProfile::Gps) 或 addRule() 设置响应规则
 *   2. setResponseDelay() 配置延迟参数
 *   3. setErrorRate() 启用错误注入
 *   4. feed(input) 发送命令并获取响应
 *   5. stats() 查看运行统计
 */
class SerialSimulator : public QObject {
    Q_OBJECT

public:
    /** @brief 构造串口设备模拟器 @param parent 父对象 */
    explicit SerialSimulator(QObject* parent = nullptr);

    // ── 响应规则管理 ──

    /** @brief 添加响应规则(pattern支持*通配符, response支持{random}/{counter}/{hex}占位符) */
    void addRule(const QByteArray& pattern, const QByteArray& response);
    /** @brief 清除所有自定义响应规则 */
    void clearRules();

    // ── 延迟配置 ──

    /** @brief 设置响应延迟范围(minMs~maxMs毫秒，实际=min+random(0,max-min)) */
    void setResponseDelay(int minMs, int maxMs);

    // ── 错误注入 ──

    /** @brief 设置错误注入率(bitErrorRate:位翻转概率, dropRate:字节丢弃概率, 范围0.0~1.0) */
    void setErrorRate(double bitErrorRate, double dropRate);

    // ── 设备配置 ──

    /** @brief 加载预定义设备配置(GPS/Modbus从站/传感器阵列/回显/自定义) */
    void loadProfile(SimulatorProfile profile);
    /** @brief 获取当前设备配置 */
    SimulatorProfile currentProfile() const;

    // ── 脚本模式 ──

    /** @brief 从JSON加载响应脚本(含"rules"数组和可选"delay"配置) @return true加载成功 */
    bool loadScript(const QJsonObject& script);

    // ── 核心交互 ──

    /** @brief 喂入请求数据并获取响应(匹配规则→生成响应→延迟→错误注入→统计更新) */
    QByteArray feed(const QByteArray& input);

    // ── 统计 ──

    /** @brief 获取运行累计统计数据 */
    const SimulatorStats& stats() const;
    /** @brief 重置累计统计计数器 */
    void resetStatistics();

signals:
    /** @brief 响应已生成 @param response 响应数据 */
    void responseGenerated(const QByteArray& response);
    /** @brief 错误已注入 @param type 错误类型("bit_error"/"dropped_byte"/"duplicated_byte"/"framing_error") */
    void errorInjected(const QString& type);

private:
    /** @brief 查找匹配的响应规则(精确优先，通配符次之) */
    const ResponseRule* findMatchingRule(const QByteArray& input) const;
    /** @brief 生成响应内容(处理{random}/{counter}/{hex}/{time}占位符) */
    QByteArray generateResponse(const QByteArray& templ) const;
    /** @brief 对响应数据注入错误(位翻转/丢弃/重复/帧错误) */
    void injectErrors(QByteArray& data);
    /** @brief 初始化GPS预设规则 */
    void setupGpsProfile();
    /** @brief 初始化Modbus从站预设规则 */
    void setupModbusSlaveProfile();
    /** @brief 初始化传感器阵列预设规则 */
    void setupSensorArrayProfile();

    QVector<ResponseRule> m_rules;                      ///< 响应规则列表
    int m_minDelayMs = 5;                               ///< 最小响应延迟(ms)
    int m_maxDelayMs = 20;                              ///< 最大响应延迟(ms)
    double m_bitErrorRate = 0.0;                        ///< 位错误概率
    double m_dropRate = 0.0;                            ///< 字节丢弃概率
    SimulatorProfile m_currentProfile = SimulatorProfile::Custom; ///< 当前设备配置
    mutable quint64 m_counter = 0;                      ///< 响应计数器({counter}占位符)
    QElapsedTimer m_timer;                              ///< 运行计时器
    qint64 m_totalDelayAccum = 0;                       ///< 累计响应延迟(用于均值)
    quint64 m_delayCount = 0;                           ///< 延迟采样次数
    SimulatorStats m_stats;                             ///< 运行累计统计
};

#endif // SERIALSIMULATOR_H

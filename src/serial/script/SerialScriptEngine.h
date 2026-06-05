/**
 * @file SerialScriptEngine.h
 * @brief 串口脚本引擎 -- 自动化串口通信的顺序执行/条件分支/变量系统
 *
 * 协作: IConnection(数据通道) / TriggerEngine(自动化配合) / ScriptRecorder(录制生成步骤)
 * 注意: 与 protocol/scriptable/ScriptableProtocolEngine 不同，本模块专注串口通信自动化。
 */
#ifndef SERIALSCRIPTENGINE_H
#define SERIALSCRIPTENGINE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QVariant>

/** @brief 脚本步骤类型 */
enum class ScriptStepType {
    Send, Receive, Wait, SetVariable, CheckCondition, GotoLabel, Log, Comment
};

/** @brief 脚本步骤结构体 — 单个步骤配置 */
struct ScriptStep {
    ScriptStepType type = ScriptStepType::Comment; ///< 步骤类型
    QString data;               ///< 数据(Send内容/Receive匹配/Log文本/SetVar值)
    int delayMs = 0;            ///< 执行后延时(毫秒)
    QString condition;          ///< 条件(CheckCondition): "var == val" / "var contains bytes" / "timeout"
    QString variable;           ///< 变量名(SetVar设置/Receive提取/条件引用)
    QString label;              ///< 标签名(GotoLabel目标/步骤标记)
    bool enabled = true;        ///< 是否启用
    int timeoutMs = 5000;       ///< Receive步骤超时(毫秒)
    static ScriptStep fromJson(const QJsonObject& json);       ///< 从JSON反序列化
    static QJsonObject toJson(const ScriptStep& step);         ///< 序列化为JSON
    static QString typeName(ScriptStepType type);              ///< 类型显示名称
};

/** @brief 脚本执行状态 */
enum class ScriptState { Idle, Running, Paused, Stepping };

/**
 * @brief 串口脚本引擎 — 解析执行脚本步骤列表，支持变量/条件分支/标签跳转/单步调试
 * 通过 dataToSend 信号驱动串口写入，通过 onReceiveData 槽接收数据。
 */
class SerialScriptEngine : public QObject {
    Q_OBJECT

public:
    explicit SerialScriptEngine(QObject* parent = nullptr);

    // ---- 脚本管理 ----
    void loadScript(const QList<ScriptStep>& steps);   ///< 加载脚本(替换当前)
    const QList<ScriptStep>& steps() const;            ///< 获取步骤列表
    void clearScript();                                ///< 清空脚本

    // ---- 执行控制 ----
    void run();        ///< 从头执行脚本
    void pause();      ///< 暂停执行
    void resume();     ///< 恢复执行
    void stop();       ///< 停止并重置
    void step();       ///< 单步执行(执行一步后暂停)
    ScriptState state() const;           ///< 当前执行状态
    int currentStepIndex() const;        ///< 当前步骤索引(-1=未运行)

    // ---- 变量系统 ----
    void setVariable(const QString& name, const QVariant& value);   ///< 设置变量
    QVariant variable(const QString& name) const;                   ///< 获取变量值
    bool hasVariable(const QString& name) const;                    ///< 变量是否存在
    void clearVariables();                                          ///< 清空变量
    QMap<QString, QVariant> allVariables() const;                   ///< 获取全部变量

    // ---- 脚本验证 ----
    QStringList validate(int maxGotoCount = 1000) const; ///< 验证脚本(无限循环/未定义标签)

    // ---- 导入/导出 ----
    QJsonObject exportToJson() const;               ///< 导出为JSON
    bool importFromJson(const QJsonObject& json);   ///< 从JSON导入

    // ---- 脚本模板 ----
    static QList<ScriptStep> createPingPongTemplate(       ///< Ping-Pong模板(发送→等回复→循环)
        const QString& sendData, const QString& expectData,
        int repeatCount = 10, int delayMs = 500);
    static QList<ScriptStep> createRetryLoopTemplate(      ///< 重试循环模板(发送→等回复→失败重试)
        const QString& sendData, const QString& expectData,
        int maxRetries = 3, int retryDelayMs = 1000);
    static QList<ScriptStep> createMultiCommandTemplate(   ///< 多命令顺序模板
        const QStringList& commands, int delayMs = 200);

    // ---- 统计 ----
    QVariantMap stats() const;      ///< 获取统计信息
    void resetStatistics();         ///< 重置统计

public slots:
    void onReceiveData(const QByteArray& data); ///< 接收串口数据(外部信号驱动)

signals:
    void scriptStarted();                               ///< 脚本开始执行
    void scriptFinished(bool success);                  ///< 脚本执行结束
    void stepExecuted(int index, const QString& type);  ///< 步骤执行完成
    void scriptError(const QString& message);           ///< 脚本错误
    void dataToSend(const QByteArray& data);            ///< 请求发送数据
    void logMessage(const QString& message);            ///< 日志输出

private:
    void executeStep(int index);            ///< 执行指定步骤
    void advanceToNextStep();               ///< 推进下一步
    int findLabelIndex(const QString& name) const;  ///< 查找标签索引
    QString resolveVariables(const QString& in) const; ///< 替换${var}引用
    bool evaluateCondition(const QString& cond) const; ///< 评估条件表达式
    void extractVariables(const QByteArray& data);     ///< 从接收数据提取变量
    void finishScript(bool success);        ///< 完成脚本执行
    void setState(ScriptState state);       ///< 设置执行状态

    QList<ScriptStep> m_steps;              ///< 脚本步骤列表
    QMap<QString, QVariant> m_variables;    ///< 变量表
    ScriptState m_state = ScriptState::Idle;///< 执行状态
    int m_currentIndex = -1;                ///< 当前步骤索引
    QByteArray m_receiveBuffer;             ///< 接收缓冲区
    QElapsedTimer m_scriptTimer;            ///< 脚本计时器
    QElapsedTimer m_stepTimer;              ///< 步骤计时器
    QTimer m_timeoutTimer;                  ///< 超时定时器
    int m_gotoCounter = 0;                  ///< 跳转计数(防无限循环)
    int m_maxGotoCount = 10000;             ///< 最大跳转次数

    quint64 m_totalScriptsExecuted = 0;     ///< 总执行脚本数
    quint64 m_totalStepsExecuted = 0;       ///< 总执行步骤数
    quint64 m_totalSendSteps = 0;           ///< 总Send步骤数
    quint64 m_totalReceiveSteps = 0;        ///< 总Receive步骤数
    quint64 m_totalConditionEvals = 0;      ///< 总条件评估次数
    quint64 m_totalBranches = 0;            ///< 总分支跳转次数
    quint64 m_totalScriptDurationMs = 0;    ///< 累计脚本时长(ms)
    quint64 m_scriptErrors = 0;             ///< 总错误数
};

#endif // SERIALSCRIPTENGINE_H

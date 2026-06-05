/**
 * @file SerialScriptEngineStats.cpp
 * @brief 串口脚本引擎 — 统计查询、重置、JSON导入导出、脚本模板
 *
 * 从 SerialScriptEngine.cpp 拆分而来，包含统计 getter、重置方法、
 * JSON序列化/反序列化和三个内置脚本模板(PingPong/重试循环/多命令顺序)。
 */

#include "serial/script/SerialScriptEngine.h"

#include <QJsonArray>

// ============================================================
// 统计查询与重置
// ============================================================

/** @brief 获取统计信息(QVariantMap形式)，包含avgScriptDurationMs平均时长计算 */
QVariantMap SerialScriptEngine::stats() const
{
    QVariantMap s;
    s["totalScriptsExecuted"] = m_totalScriptsExecuted;
    s["totalStepsExecuted"] = m_totalStepsExecuted;
    s["totalSendSteps"] = m_totalSendSteps;
    s["totalReceiveSteps"] = m_totalReceiveSteps;
    s["totalConditionEvals"] = m_totalConditionEvals;
    s["totalBranches"] = m_totalBranches;
    s["avgScriptDurationMs"] = m_totalScriptsExecuted > 0
        ? static_cast<double>(m_totalScriptDurationMs) / m_totalScriptsExecuted : 0.0;
    s["scriptErrors"] = m_scriptErrors;
    return s;
}

/** @brief 重置所有统计计数器为初始值(不影响脚本内容、变量和执行状态) */
void SerialScriptEngine::resetStatistics()
{
    m_totalScriptsExecuted = 0;
    m_totalStepsExecuted = 0;
    m_totalSendSteps = 0;
    m_totalReceiveSteps = 0;
    m_totalConditionEvals = 0;
    m_totalBranches = 0;
    m_totalScriptDurationMs = 0;
    m_scriptErrors = 0;
}

// ============================================================
// JSON 导入 / 导出
// ============================================================

/** @brief 将当前脚本和变量导出为JSON对象(版本号=1) */
QJsonObject SerialScriptEngine::exportToJson() const
{
    QJsonObject root;
    root["version"] = 1;
    QJsonArray arr;
    for (const auto& s : m_steps) {
        arr.append(ScriptStep::toJson(s));
    }
    root["steps"] = arr;
    /* 导出变量表 */
    QJsonObject vars;
    for (auto it = m_variables.constBegin(); it != m_variables.constEnd(); ++it) {
        vars[it.key()] = QJsonValue::fromVariant(it.value());
    }
    root["variables"] = vars;
    return root;
}

/** @brief 从JSON对象导入脚本和变量(替换当前内容)，版本号必须为1 */
bool SerialScriptEngine::importFromJson(const QJsonObject& json)
{
    if (json["version"].toInt(0) != 1) return false;
    QJsonArray arr = json["steps"].toArray();
    QList<ScriptStep> steps;
    for (const auto& val : arr) {
        steps.append(ScriptStep::fromJson(val.toObject()));
    }
    loadScript(steps);
    /* 导入变量 */
    QJsonObject vars = json["variables"].toObject();
    for (auto it = vars.constBegin(); it != vars.constEnd(); ++it) {
        m_variables[it.key()] = it.value().toVariant();
    }
    return true;
}

// ============================================================
// 脚本模板
// ============================================================

/**
 * @brief 创建Ping-Pong模板脚本(发送→等待回复→校验→循环)
 *
 * 流程: Log(开始) → Comment(loop_start) → Send → Receive(timeout条件) →
 *       CheckCondition(timeout→loop_start) → Log(完成)
 * @param sendData 发送数据(Hex格式)
 * @param expectData 期望回复(Hex格式)
 * @param repeatCount 重复次数(0=无限循环直到手动停止)
 * @param delayMs 每次发送间隔(毫秒)
 */
QList<ScriptStep> SerialScriptEngine::createPingPongTemplate(
    const QString& sendData, const QString& expectData, int repeatCount, int delayMs)
{
    QList<ScriptStep> steps;
    steps.append({ScriptStepType::Log, QStringLiteral("Ping-Pong开始，重复%1次").arg(repeatCount),
                  0, {}, {}, {}, true, 5000});
    steps.append({ScriptStepType::Comment, QStringLiteral("循环起点"),
                  0, {}, {}, QStringLiteral("loop_start"), true, 5000});
    steps.append({ScriptStepType::Send, sendData, 0, {}, {}, {}, true, 5000});
    steps.append({ScriptStepType::Receive, expectData, delayMs,
                  QStringLiteral("timeout"), QStringLiteral("reply"), {}, true, 5000});
    if (repeatCount > 0) {
        steps.append({ScriptStepType::CheckCondition, {},
                      0, QStringLiteral("timeout"), {}, QStringLiteral("loop_start"), true, 5000});
    }
    steps.append({ScriptStepType::Log, QStringLiteral("Ping-Pong完成"), 0, {}, {}, {}, true, 5000});
    return steps;
}

/**
 * @brief 创建重试循环模板(发送→等待→失败重试)
 *
 * 流程: SetVariable(retry_count=0) → Comment(retry_start) → Send → Receive →
 *       Log(成功) → GotoLabel(done) → Comment(retry_check) →
 *       SetVariable(retry_count++) → CheckCondition(retry_count==max → done) →
 *       GotoLabel(retry_start) → Comment(done) → Log(结束)
 */
QList<ScriptStep> SerialScriptEngine::createRetryLoopTemplate(
    const QString& sendData, const QString& expectData, int maxRetries, int retryDelayMs)
{
    QList<ScriptStep> steps;
    steps.append({ScriptStepType::SetVariable, QString::number(0),
                  0, {}, QStringLiteral("retry_count"), {}, true, 5000});
    steps.append({ScriptStepType::Comment, QStringLiteral("重试起点"),
                  0, {}, {}, QStringLiteral("retry_start"), true, 5000});
    steps.append({ScriptStepType::Send, sendData, 0, {}, {}, {}, true, 5000});
    steps.append({ScriptStepType::Receive, expectData, 0,
                  QStringLiteral("timeout"), QStringLiteral("reply"), {}, true, 5000});
    steps.append({ScriptStepType::Log, QStringLiteral("收到回复，成功"), 0, {}, {}, {}, true, 5000});
    steps.append({ScriptStepType::GotoLabel, {}, 0, {}, {}, QStringLiteral("done"), true, 5000});
    steps.append({ScriptStepType::Comment, QStringLiteral("重试检查"),
                  0, {}, {}, QStringLiteral("retry_check"), true, 5000});
    steps.append({ScriptStepType::SetVariable, {}, retryDelayMs,
                  {}, QStringLiteral("retry_count"), {}, true, 5000});
    steps.append({ScriptStepType::CheckCondition, {},
                  0, QStringLiteral("retry_count == %1").arg(maxRetries),
                  {}, QStringLiteral("done"), true, 5000});
    steps.append({ScriptStepType::GotoLabel, {}, 0, {}, {}, QStringLiteral("retry_start"), true, 5000});
    steps.append({ScriptStepType::Comment, QStringLiteral("结束"),
                  0, {}, {}, QStringLiteral("done"), true, 5000});
    steps.append({ScriptStepType::Log, QStringLiteral("重试循环结束"), 0, {}, {}, {}, true, 5000});
    return steps;
}

/**
 * @brief 创建多命令顺序执行模板
 *
 * 每条命令生成一个Send步骤(带delayMs间隔)，最后追加一条Log步骤。
 * @param commands 命令字符串列表
 * @param delayMs 命令间间隔(毫秒)
 */
QList<ScriptStep> SerialScriptEngine::createMultiCommandTemplate(
    const QStringList& commands, int delayMs)
{
    QList<ScriptStep> steps;
    for (const auto& cmd : commands) {
        steps.append({ScriptStepType::Send, cmd, delayMs, {}, {}, {}, true, 5000});
    }
    steps.append({ScriptStepType::Log, QStringLiteral("多命令序列执行完毕"), 0, {}, {}, {}, true, 5000});
    return steps;
}

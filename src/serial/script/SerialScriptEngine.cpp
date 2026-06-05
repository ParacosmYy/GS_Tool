/**
 * @file SerialScriptEngine.cpp
 * @brief 串口脚本引擎实现 — 顺序执行/条件分支/变量系统/单步调试
 *
 * 核心执行逻辑: 步骤执行、变量系统、条件评估、接收数据匹配。
 * 统计/模板/序列化见 SerialScriptEngineStats.cpp 和 SerialScriptEngineTemplates.cpp。
 */

#include "serial/script/SerialScriptEngine.h"

#include <QRegularExpression>

// ============================================================
// ScriptStep 序列化
// ============================================================

/** @brief 从JSON反序列化步骤 */
ScriptStep ScriptStep::fromJson(const QJsonObject& json)
{
    ScriptStep s;
    s.type = static_cast<ScriptStepType>(json["type"].toInt(static_cast<int>(ScriptStepType::Comment)));
    s.data = json["data"].toString();
    s.delayMs = json["delayMs"].toInt(0);
    s.condition = json["condition"].toString();
    s.variable = json["variable"].toString();
    s.label = json["label"].toString();
    s.enabled = json["enabled"].toBool(true);
    s.timeoutMs = json["timeoutMs"].toInt(5000);
    return s;
}

/** @brief 序列化步骤为JSON */
QJsonObject ScriptStep::toJson(const ScriptStep& step)
{
    QJsonObject o;
    o["type"] = static_cast<int>(step.type);
    o["data"] = step.data;
    o["delayMs"] = step.delayMs;
    o["condition"] = step.condition;
    o["variable"] = step.variable;
    o["label"] = step.label;
    o["enabled"] = step.enabled;
    o["timeoutMs"] = step.timeoutMs;
    return o;
}

/** @brief 获取步骤类型显示名称 */
QString ScriptStep::typeName(ScriptStepType type)
{
    switch (type) {
    case ScriptStepType::Send:           return QStringLiteral("Send");
    case ScriptStepType::Receive:        return QStringLiteral("Receive");
    case ScriptStepType::Wait:           return QStringLiteral("Wait");
    case ScriptStepType::SetVariable:    return QStringLiteral("SetVariable");
    case ScriptStepType::CheckCondition: return QStringLiteral("CheckCondition");
    case ScriptStepType::GotoLabel:      return QStringLiteral("GotoLabel");
    case ScriptStepType::Log:            return QStringLiteral("Log");
    case ScriptStepType::Comment:        return QStringLiteral("Comment");
    }
    return QStringLiteral("Unknown");
}

// ============================================================
// 构造 / 脚本管理
// ============================================================

/** @brief 构造脚本引擎，初始化超时定时器 */
SerialScriptEngine::SerialScriptEngine(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SerialScriptEngine"));
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, [this]() {
        /* Receive步骤超时 → 检查是否有timeout条件处理 */
        if (m_state == ScriptState::Running || m_state == ScriptState::Stepping) {
            bool handled = false;
            if (m_currentIndex >= 0 && m_currentIndex < m_steps.size()) {
                const auto& step = m_steps[m_currentIndex];
                if (step.type == ScriptStepType::Receive && step.condition.contains("timeout")) {
                    handled = evaluateCondition("timeout");
                }
            }
            if (!handled) {
                ++m_scriptErrors;
                emit scriptError(tr("Receive步骤超时(index=%1)").arg(m_currentIndex));
                finishScript(false);
            }
        }
    });
}

void SerialScriptEngine::loadScript(const QList<ScriptStep>& steps) { m_steps = steps; }
const QList<ScriptStep>& SerialScriptEngine::steps() const { return m_steps; }
void SerialScriptEngine::clearScript() { m_steps.clear(); stop(); }

// ============================================================
// 执行控制
// ============================================================

/** @brief 从头执行脚本 */
void SerialScriptEngine::run()
{
    if (m_steps.isEmpty()) { emit scriptError(tr("脚本为空")); return; }
    m_currentIndex = 0;
    m_gotoCounter = 0;
    m_receiveBuffer.clear();
    m_scriptTimer.start();
    setState(ScriptState::Running);
    ++m_totalScriptsExecuted;
    emit scriptStarted();
    executeStep(m_currentIndex);
}

void SerialScriptEngine::pause()
{
    if (m_state == ScriptState::Running || m_state == ScriptState::Stepping) {
        m_timeoutTimer.stop();
        setState(ScriptState::Paused);
    }
}

void SerialScriptEngine::resume()
{
    if (m_state == ScriptState::Paused && m_currentIndex >= 0) {
        setState(ScriptState::Running);
        executeStep(m_currentIndex);
    }
}

void SerialScriptEngine::stop()
{
    m_timeoutTimer.stop();
    if (m_state != ScriptState::Idle) finishScript(m_currentIndex >= m_steps.size());
    m_currentIndex = -1;
    setState(ScriptState::Idle);
}

/** @brief 单步执行 */
void SerialScriptEngine::step()
{
    if (m_state == ScriptState::Idle) {
        if (m_steps.isEmpty()) { emit scriptError(tr("脚本为空")); return; }
        m_currentIndex = 0;
        m_gotoCounter = 0;
        m_receiveBuffer.clear();
        m_scriptTimer.start();
        ++m_totalScriptsExecuted;
        emit scriptStarted();
    }
    setState(ScriptState::Stepping);
    executeStep(m_currentIndex);
}

ScriptState SerialScriptEngine::state() const { return m_state; }
int SerialScriptEngine::currentStepIndex() const { return m_currentIndex; }

// ============================================================
// 变量系统
// ============================================================

void SerialScriptEngine::setVariable(const QString& name, const QVariant& value) { m_variables[name] = value; }
QVariant SerialScriptEngine::variable(const QString& name) const { return m_variables.value(name); }
bool SerialScriptEngine::hasVariable(const QString& name) const { return m_variables.contains(name); }
void SerialScriptEngine::clearVariables() { m_variables.clear(); }
QMap<QString, QVariant> SerialScriptEngine::allVariables() const { return m_variables; }

/** @brief 替换字符串中的 ${varName} 变量引用 */
QString SerialScriptEngine::resolveVariables(const QString& input) const
{
    QString result = input;
    QRegularExpression re(QStringLiteral(R"(\$\{(\w+)\})"));
    QRegularExpressionMatchIterator it = re.globalMatch(input);
    while (it.hasNext()) {
        auto match = it.next();
        QString varName = match.captured(1);
        if (m_variables.contains(varName))
            result.replace(match.captured(0), m_variables[varName].toString());
    }
    return result;
}

/** @brief 从接收数据中提取变量 */
void SerialScriptEngine::extractVariables(const QByteArray& data)
{
    if (m_currentIndex < 0 || m_currentIndex >= m_steps.size()) return;
    const auto& step = m_steps[m_currentIndex];
    if (step.variable.isEmpty()) return;
    QString text = QString::fromUtf8(data);
    if (step.data.isEmpty()) {
        m_variables[step.variable] = text.trimmed();
    } else {
        QRegularExpression re(step.data);
        if (re.isValid()) {
            auto match = re.match(text);
            if (match.hasMatch()) {
                m_variables[step.variable] = match.captured(1).isEmpty()
                    ? match.captured(0) : match.captured(1);
            }
        }
    }
}

/** @brief 评估条件: "var == value" / "var contains bytes" / "timeout" */
bool SerialScriptEngine::evaluateCondition(const QString& condition) const
{
    ++const_cast<quint64&>(m_totalConditionEvals);
    QString cond = condition.trimmed();
    if (cond == "timeout") return true;

    QRegularExpression eqRe(R"(^\s*(\w+)\s*==\s*(.+)$)");
    auto eqMatch = eqRe.match(cond);
    if (eqMatch.hasMatch()) {
        QString varName = eqMatch.captured(1);
        if (!m_variables.contains(varName)) return false;
        return m_variables[varName].toString() == eqMatch.captured(2).trimmed();
    }

    QRegularExpression containsRe(R"(^\s*(\w+)\s+contains\s+(.+)$)");
    auto cMatch = containsRe.match(cond);
    if (cMatch.hasMatch()) {
        QString varName = cMatch.captured(1);
        if (!m_variables.contains(varName)) return false;
        return m_variables[varName].toString().contains(cMatch.captured(2).trimmed());
    }
    return false;
}

// ============================================================
// 核心: 步骤执行
// ============================================================

/** @brief 执行指定索引的步骤 */
void SerialScriptEngine::executeStep(int index)
{
    if (index < 0 || index >= m_steps.size()) { finishScript(true); return; }
    const auto& step = m_steps[index];
    if (!step.enabled || step.type == ScriptStepType::Comment) {
        m_currentIndex = index + 1;
        advanceToNextStep();
        return;
    }
    ++m_totalStepsExecuted;
    m_stepTimer.start();

    switch (step.type) {
    case ScriptStepType::Send: {
        ++m_totalSendSteps;
        emit dataToSend(resolveVariables(step.data).toUtf8());
        emit stepExecuted(index, ScriptStep::typeName(step.type));
        m_currentIndex = index + 1;
        if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
        else advanceToNextStep();
        break;
    }
    case ScriptStepType::Receive: {
        ++m_totalReceiveSteps;
        QString bufStr = QString::fromUtf8(m_receiveBuffer);
        QString pattern = resolveVariables(step.data);
        if (!pattern.isEmpty() && bufStr.contains(pattern)) {
            extractVariables(m_receiveBuffer);
            m_receiveBuffer.clear();
            emit stepExecuted(index, ScriptStep::typeName(step.type));
            m_currentIndex = index + 1;
            if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
            else advanceToNextStep();
        } else {
            m_timeoutTimer.start(step.timeoutMs);
        }
        break;
    }
    case ScriptStepType::Wait: {
        emit stepExecuted(index, ScriptStep::typeName(step.type));
        m_currentIndex = index + 1;
        QTimer::singleShot(qMax(step.delayMs, 1), this, [this]() { advanceToNextStep(); });
        break;
    }
    case ScriptStepType::SetVariable: {
        QString value = resolveVariables(step.data);
        bool isNum = false;
        double numVal = value.toDouble(&isNum);
        m_variables[step.variable] = isNum ? QVariant(numVal) : QVariant(value);
        emit stepExecuted(index, ScriptStep::typeName(step.type));
        m_currentIndex = index + 1;
        if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
        else advanceToNextStep();
        break;
    }
    case ScriptStepType::CheckCondition: {
        bool result = evaluateCondition(step.condition);
        emit stepExecuted(index, ScriptStep::typeName(step.type));
        if (result && !step.label.isEmpty()) {
            int target = findLabelIndex(step.label);
            if (target >= 0) {
                ++m_totalBranches; ++m_gotoCounter;
                if (m_gotoCounter > m_maxGotoCount) {
                    ++m_scriptErrors;
                    emit scriptError(tr("跳转次数超限(>%1)，疑似无限循环").arg(m_maxGotoCount));
                    finishScript(false); return;
                }
                m_currentIndex = target;
            } else {
                ++m_scriptErrors;
                emit scriptError(tr("未定义标签: %1").arg(step.label));
                finishScript(false); return;
            }
        } else { m_currentIndex = index + 1; }
        if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
        else advanceToNextStep();
        break;
    }
    case ScriptStepType::GotoLabel: {
        ++m_totalBranches; ++m_gotoCounter;
        if (m_gotoCounter > m_maxGotoCount) {
            ++m_scriptErrors;
            emit scriptError(tr("跳转次数超限(>%1)，疑似无限循环").arg(m_maxGotoCount));
            finishScript(false); return;
        }
        int target = findLabelIndex(step.label);
        if (target >= 0) {
            emit stepExecuted(index, ScriptStep::typeName(step.type));
            m_currentIndex = target;
        } else {
            ++m_scriptErrors;
            emit scriptError(tr("未定义标签: %1").arg(step.label));
            finishScript(false); return;
        }
        if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
        else advanceToNextStep();
        break;
    }
    case ScriptStepType::Log: {
        emit logMessage(resolveVariables(step.data));
        emit stepExecuted(index, ScriptStep::typeName(step.type));
        m_currentIndex = index + 1;
        if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
        else advanceToNextStep();
        break;
    }
    case ScriptStepType::Comment:
        m_currentIndex = index + 1;
        advanceToNextStep();
        break;
    }
}

void SerialScriptEngine::advanceToNextStep()
{
    if (m_state == ScriptState::Paused) return;
    if (m_state == ScriptState::Stepping) { setState(ScriptState::Paused); return; }
    if (m_currentIndex >= m_steps.size()) { finishScript(true); return; }
    executeStep(m_currentIndex);
}

int SerialScriptEngine::findLabelIndex(const QString& labelName) const
{
    for (int i = 0; i < m_steps.size(); ++i) {
        if (m_steps[i].label == labelName) return i;
    }
    return -1;
}

/** @brief 接收串口数据，匹配Receive步骤 */
void SerialScriptEngine::onReceiveData(const QByteArray& data)
{
    if (m_state != ScriptState::Running && m_state != ScriptState::Stepping) return;
    if (m_currentIndex < 0 || m_currentIndex >= m_steps.size()) return;
    const auto& step = m_steps[m_currentIndex];
    if (step.type == ScriptStepType::Receive) {
        m_receiveBuffer.append(data);
        QString bufStr = QString::fromUtf8(m_receiveBuffer);
        QString pattern = resolveVariables(step.data);
        if (pattern.isEmpty() || bufStr.contains(pattern)) {
            m_timeoutTimer.stop();
            extractVariables(m_receiveBuffer);
            m_receiveBuffer.clear();
            emit stepExecuted(m_currentIndex, ScriptStep::typeName(step.type));
            m_currentIndex = m_currentIndex + 1;
            if (step.delayMs > 0) QTimer::singleShot(step.delayMs, this, [this]() { advanceToNextStep(); });
            else advanceToNextStep();
        }
    }
}

void SerialScriptEngine::finishScript(bool success)
{
    m_timeoutTimer.stop();
    m_totalScriptDurationMs += static_cast<quint64>(m_scriptTimer.elapsed());
    setState(ScriptState::Idle);
    emit scriptFinished(success);
}

void SerialScriptEngine::setState(ScriptState state) { m_state = state; }

// ============================================================
// 脚本验证
// ============================================================

/** @brief 验证脚本: 检查未定义标签、重复标签、无限循环 */
QStringList SerialScriptEngine::validate(int maxGotoCount) const
{
    QStringList errors;
    QSet<QString> definedLabels, referencedLabels;
    for (int i = 0; i < m_steps.size(); ++i) {
        const auto& s = m_steps[i];
        if (!s.label.isEmpty()) {
            if (definedLabels.contains(s.label))
                errors << tr("步骤%1: 重复标签'%2'").arg(i).arg(s.label);
            definedLabels.insert(s.label);
        }
        if ((s.type == ScriptStepType::GotoLabel || s.type == ScriptStepType::CheckCondition)
            && !s.label.isEmpty()) {
            referencedLabels.insert(s.label);
        } else if (s.type == ScriptStepType::GotoLabel && s.label.isEmpty()) {
            errors << tr("步骤%1: GotoLabel未指定目标标签").arg(i);
        }
    }
    for (const auto& ref : referencedLabels) {
        if (!definedLabels.contains(ref))
            errors << tr("未定义标签: '%1'").arg(ref);
    }
    /* 模拟执行检测无限循环 */
    int idx = 0, gotos = 0;
    while (idx >= 0 && idx < m_steps.size() && gotos < maxGotoCount) {
        if (m_steps[idx].type == ScriptStepType::GotoLabel) {
            int target = findLabelIndex(m_steps[idx].label);
            if (target < 0) break;
            idx = target; ++gotos;
        } else { ++idx; }
    }
    if (gotos >= maxGotoCount)
        errors << tr("模拟执行超过%1次跳转，可能存在无限循环").arg(maxGotoCount);
    return errors;
}

// 导入/导出和统计见 SerialScriptEngineStats.cpp

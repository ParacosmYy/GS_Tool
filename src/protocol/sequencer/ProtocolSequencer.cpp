/**
 * @file ProtocolSequencer.cpp
 * @brief 协议序列器引擎 -- 核心执行逻辑与 JSON 序列化
 *
 * 从 ProtocolSequencer.h 拆分出的主实现，职责:
 *   1. 构造/析构与定时器初始化
 *   2. 序列管理(setSequence/loadSequence/saveSequence)
 *   3. 文件 I/O(loadFromFile/saveToFile)
 *   4. 执行引擎(execute/stop/pause/resume)
 *   5. 各步骤类型的执行逻辑
 *   6. JSON 序列化辅助
 */

#include "protocol/sequencer/ProtocolSequencer.h"

#include <QFile>
#include <QJsonDocument>
#include <QRegularExpression>

// ============================================================================
// 构造 / 析构
// ============================================================================

ProtocolSequencer::ProtocolSequencer(QObject* parent)
    : QObject(parent)
    , m_delayTimer(new QTimer(this))
    , m_receiveTimeoutTimer(new QTimer(this))
{
    m_delayTimer->setSingleShot(true);
    m_receiveTimeoutTimer->setSingleShot(true);

    connect(m_delayTimer, &QTimer::timeout, this, [this]() {
        if (m_running && !m_paused) {
            ++m_currentIndex;
            executeNextStep();
        }
    });

    connect(m_receiveTimeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_running) {
            emit logMessage(tr("Receive step timed out at index %1").arg(m_currentIndex));
            finishSequence(false, tr("Receive timeout at step %1").arg(m_currentIndex));
        }
    });
}

ProtocolSequencer::~ProtocolSequencer()
{
    stop();
}

// ============================================================================
// 序列管理
// ============================================================================

void ProtocolSequencer::setSequence(const QList<SequenceStep>& steps)
{
    if (m_running) { stop(); }
    m_steps = steps;
}

QList<SequenceStep> ProtocolSequencer::sequence() const
{
    return m_steps;
}

// ============================================================================
// JSON 加载/保存
// ============================================================================

bool ProtocolSequencer::loadSequence(const QJsonObject& json)
{
    if (!json.contains("steps")) { return false; }
    QJsonArray arr = json["steps"].toArray();
    QList<SequenceStep> steps;
    steps.reserve(arr.size());
    for (const QJsonValue& val : arr) {
        steps.append(jsonToStep(val.toObject()));
    }
    setSequence(steps);
    return true;
}

QJsonObject ProtocolSequencer::saveSequence() const
{
    QJsonArray arr;
    for (const auto& step : m_steps) {
        arr.append(stepToJson(step));
    }
    return QJsonObject{{"steps", arr}};
}

bool ProtocolSequencer::loadFromFile(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) { return false; }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) { return false; }
    return loadSequence(doc.object());
}

bool ProtocolSequencer::saveToFile(const QString& filePath) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly)) { return false; }
    f.write(QJsonDocument(saveSequence()).toJson(QJsonDocument::Indented));
    return true;
}

// ============================================================================
// 执行控制
// ============================================================================

void ProtocolSequencer::execute()
{
    if (m_steps.isEmpty()) {
        emit sequenceComplete(SequenceResult{false, 0, 0, 0, tr("Empty sequence")});
        return;
    }
    if (m_running) { stop(); }

    m_running = true;
    m_paused = false;
    m_currentIndex = 0;
    m_receivedBuffer.clear();
    m_loopStack.clear();
    m_currentResult = SequenceResult{};
    m_sequenceTimer.start();

    ++m_stats.sequencesExecuted;
    emit logMessage(tr("Sequence execution started (%1 steps)").arg(m_steps.size()));
    executeNextStep();
}

void ProtocolSequencer::stop()
{
    m_delayTimer->stop();
    m_receiveTimeoutTimer->stop();
    if (m_running) {
        m_running = false;
        m_paused = false;
        emit logMessage(tr("Sequence execution stopped"));
    }
}

void ProtocolSequencer::pause()
{
    if (m_running && !m_paused) {
        m_paused = true;
        m_delayTimer->stop();
        emit logMessage(tr("Sequence execution paused at step %1").arg(m_currentIndex));
    }
}

void ProtocolSequencer::resume()
{
    if (m_running && m_paused) {
        m_paused = false;
        emit logMessage(tr("Sequence execution resumed"));
        executeNextStep();
    }
}

bool ProtocolSequencer::isRunning() const { return m_running; }
bool ProtocolSequencer::isPaused() const { return m_paused; }

// ============================================================================
// 数据输入
// ============================================================================

void ProtocolSequencer::feedReceivedData(const QByteArray& data)
{
    m_receivedBuffer.append(data);
    m_stats.bytesReceived += static_cast<quint64>(data.size());
}

// ============================================================================
// 内部执行引擎
// ============================================================================

void ProtocolSequencer::executeNextStep()
{
    if (!m_running || m_paused) { return; }

    /* 循环栈回绕检查 */
    if (!m_loopStack.isEmpty()) {
        auto& top = m_loopStack.last();
        if (m_currentIndex > top.endIndex) {
            --top.remaining;
            if (top.remaining > 0) {
                m_currentIndex = top.startIndex;
            } else {
                m_loopStack.removeLast();
            }
        }
    }

    if (m_currentIndex < 0 || m_currentIndex >= m_steps.size()) {
        finishSequence(true);
        return;
    }

    const SequenceStep& step = m_steps[m_currentIndex];
    emit stepStarted(m_currentIndex);

    switch (step.type) {
    case StepType::Send:    executeSendStep(step);    break;
    case StepType::Receive: executeReceiveStep(step); break;
    case StepType::Delay:   executeDelayStep(step);   break;
    case StepType::Loop:    executeLoopStep(step);    break;
    case StepType::Check:   executeCheckStep(step);   break;
    case StepType::WaitFor: executeReceiveStep(step); break;
    case StepType::Branch:
        emit logMessage(tr("Branch step at %1 — treating as pass-through").arg(m_currentIndex));
        ++m_currentIndex;
        executeNextStep();
        break;
    }
}

void ProtocolSequencer::executeSendStep(const SequenceStep& step)
{
    emit sendData(step.data);
    m_stats.bytesSent += static_cast<quint64>(step.data.size());
    emit stepCompleted(m_currentIndex, true);
    emit logMessage(tr("Send %1 bytes at step %2").arg(step.data.size()).arg(m_currentIndex));

    ++m_currentResult.stepsCompleted;
    ++m_stats.stepsCompleted;

    if (step.delayMs > 0) {
        m_delayTimer->start(step.delayMs);
    } else {
        ++m_currentIndex;
        executeNextStep();
    }
}

void ProtocolSequencer::executeReceiveStep(const SequenceStep& step)
{
    int timeout = (step.delayMs > 0) ? step.delayMs : 5000;

    /* 检查缓冲区是否已有匹配数据 */
    if (!step.expectedPattern.isEmpty() && m_receivedBuffer.contains(step.expectedPattern.toUtf8())) {
        emit stepCompleted(m_currentIndex, true);
        emit logMessage(tr("Receive matched at step %1").arg(m_currentIndex));
        m_receivedBuffer.clear();
        ++m_currentResult.stepsCompleted;
        ++m_stats.stepsCompleted;
        ++m_currentIndex;
        executeNextStep();
        return;
    }

    m_receiveTimeoutTimer->start(timeout);
    /* 连接一次性检查: 超时前若数据到达，由 QTimer 或外部 feed 触发 */
    disconnect(m_receiveTimeoutTimer, nullptr, nullptr, nullptr);
    connect(m_receiveTimeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_running) {
            emit stepCompleted(m_currentIndex, false);
            ++m_currentResult.errorsFound;
            ++m_stats.stepsFailed;
            finishSequence(false, tr("Receive timeout at step %1").arg(m_currentIndex));
        }
    }, Qt::UniqueConnection);
}

void ProtocolSequencer::executeDelayStep(const SequenceStep& step)
{
    int ms = (step.delayMs > 0) ? step.delayMs : 100;
    emit logMessage(tr("Delay %1 ms at step %2").arg(ms).arg(m_currentIndex));
    m_delayTimer->start(ms);
}

void ProtocolSequencer::executeLoopStep(const SequenceStep& step)
{
    if (step.subSteps.isEmpty() || step.loopCount <= 0) {
        emit stepCompleted(m_currentIndex, true);
        ++m_currentResult.stepsCompleted;
        ++m_stats.stepsCompleted;
        ++m_currentIndex;
        executeNextStep();
        return;
    }

    /* 展开子步骤到主序列中 */
    int insertPos = m_currentIndex + 1;
    for (int i = 0; i < step.loopCount; ++i) {
        for (int j = 0; j < step.subSteps.size(); ++j) {
            m_steps.insert(insertPos++, step.subSteps[j]);
        }
    }

    LoopState ls;
    ls.startIndex = m_currentIndex + 1;
    ls.endIndex = ls.startIndex + (step.subSteps.size() * step.loopCount) - 1;
    ls.remaining = 1; /* 已展开，只走一次 */
    m_loopStack.append(ls);

    emit stepCompleted(m_currentIndex, true);
    ++m_currentResult.stepsCompleted;
    ++m_stats.stepsCompleted;
    ++m_currentIndex;
    executeNextStep();
}

void ProtocolSequencer::executeCheckStep(const SequenceStep& step)
{
    bool ok = true;
    if (!step.expectedPattern.isEmpty()) {
        ok = m_receivedBuffer.contains(step.expectedPattern.toUtf8());
    } else if (!step.data.isEmpty()) {
        ok = (m_receivedBuffer == step.data);
    }

    emit stepCompleted(m_currentIndex, ok);
    if (ok) {
        ++m_currentResult.stepsCompleted;
        ++m_stats.stepsCompleted;
    } else {
        ++m_currentResult.errorsFound;
        ++m_stats.stepsFailed;
    }
    m_receivedBuffer.clear();
    ++m_currentIndex;
    executeNextStep();
}

void ProtocolSequencer::finishSequence(bool success, const QString& error)
{
    m_running = false;
    m_paused = false;
    m_delayTimer->stop();
    m_receiveTimeoutTimer->stop();

    m_currentResult.success = success;
    m_currentResult.durationMs = m_sequenceTimer.elapsed();
    if (!error.isEmpty()) {
        m_currentResult.errorMessage = error;
    }

    m_stats.totalDurationMs += static_cast<quint64>(m_currentResult.durationMs);
    if (success) {
        ++m_stats.sequencesSucceeded;
    } else {
        ++m_stats.sequencesFailed;
    }

    emit sequenceComplete(m_currentResult);
    emit logMessage(tr("Sequence %1 in %2 ms (%3 steps, %4 errors")
        .arg(success ? tr("succeeded") : tr("failed"))
        .arg(m_currentResult.durationMs)
        .arg(m_currentResult.stepsCompleted)
        .arg(m_currentResult.errorsFound));
}

// ============================================================================
// JSON 序列化辅助
// ============================================================================

QJsonObject ProtocolSequencer::stepToJson(const SequenceStep& step) const
{
    QJsonObject obj;
    obj["type"] = stepTypeToString(step.type);
    obj["data"] = QString(step.data.toHex());
    obj["delayMs"] = step.delayMs;
    obj["description"] = step.description;
    obj["expectedPattern"] = step.expectedPattern;
    obj["loopCount"] = step.loopCount;

    if (!step.subSteps.isEmpty()) {
        QJsonArray arr;
        for (const auto& sub : step.subSteps) {
            arr.append(stepToJson(sub));
        }
        obj["subSteps"] = arr;
    }
    return obj;
}

SequenceStep ProtocolSequencer::jsonToStep(const QJsonObject& obj) const
{
    SequenceStep step;
    step.type = stringToStepType(obj["type"].toString("send"));
    step.data = QByteArray::fromHex(obj["data"].toString().toUtf8());
    step.delayMs = obj["delayMs"].toInt(0);
    step.description = obj["description"].toString();
    step.expectedPattern = obj["expectedPattern"].toString();
    step.loopCount = obj["loopCount"].toInt(1);

    if (obj.contains("subSteps")) {
        QJsonArray arr = obj["subSteps"].toArray();
        step.subSteps.reserve(arr.size());
        for (const QJsonValue& val : arr) {
            step.subSteps.append(jsonToStep(val.toObject()));
        }
    }
    return step;
}

QString ProtocolSequencer::stepTypeToString(StepType type)
{
    switch (type) {
    case StepType::Send:    return "send";
    case StepType::Receive: return "receive";
    case StepType::Delay:   return "delay";
    case StepType::WaitFor: return "waitfor";
    case StepType::Check:   return "check";
    case StepType::Loop:    return "loop";
    case StepType::Branch:  return "branch";
    }
    return "send";
}

StepType ProtocolSequencer::stringToStepType(const QString& str)
{
    QString s = str.toLower();
    if (s == "receive")  return StepType::Receive;
    if (s == "delay")    return StepType::Delay;
    if (s == "waitfor")  return StepType::WaitFor;
    if (s == "check")    return StepType::Check;
    if (s == "loop")     return StepType::Loop;
    if (s == "branch")   return StepType::Branch;
    return StepType::Send;
}

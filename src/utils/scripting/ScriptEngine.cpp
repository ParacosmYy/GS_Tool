/**
 * @file ScriptEngine.cpp
 * @brief 脚本执行引擎实现 — QJSEngine 沙箱、辅助函数注入、编译缓存、超时保护
 *
 * 核心执行: 编译→参数替换→注入上下文→同步执行→捕获结果。
 * 统计见 ScriptEngineStats.cpp。
 */

#include "utils/scripting/ScriptEngine.h"

#include <QCryptographicHash>
#include <QRegularExpression>

// ============================================================
// ScriptAction 序列化
// ============================================================

/** @brief 从 QVariantMap 反序列化 ScriptAction */
ScriptAction ScriptAction::fromVariantMap(const QMap<QString, QVariant>& map)
{
    ScriptAction a;
    a.name = map["name"].toString();
    a.code = map["code"].toString();
    a.language = static_cast<ScriptLanguage>(map["language"].toInt(0));
    a.triggers = map["triggers"].toStringList();
    a.enabled = map.contains("enabled") ? map["enabled"].toBool() : true;

    const QVariantMap pm = map["params"].toMap();
    for (auto it = pm.constBegin(); it != pm.constEnd(); ++it)
        a.params[it.key()] = it.value().toString();
    return a;
}

/** @brief 序列化 ScriptAction 为 QVariantMap */
QMap<QString, QVariant> ScriptAction::toVariantMap() const
{
    QMap<QString, QVariant> m;
    m["name"] = name;
    m["code"] = code;
    m["language"] = static_cast<int>(language);
    m["triggers"] = triggers;
    m["enabled"] = enabled;
    QVariantMap pm;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it)
        pm[it.key()] = it.value();
    m["params"] = pm;
    return m;
}

// ============================================================
// ScriptResult 工厂
// ============================================================

ScriptResult ScriptResult::ok(const QString& out, qint64 dur, const QVariant& ret)
{
    return {true, out, {}, -1, dur, ret};
}

ScriptResult ScriptResult::fail(const QString& err, int ln, qint64 dur)
{
    return {false, {}, err, ln, dur, {}};
}

// ============================================================
// 构造
// ============================================================

/** @brief 构造脚本引擎，配置全局引擎属性 */
ScriptEngine::ScriptEngine(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ScriptEngine"));
}

// ============================================================
// 辅助函数注入
// ============================================================

/** @brief 向 QJSEngine 注入 serialRead/serialWrite/log/helper 函数 */
void ScriptEngine::injectHelpers(QJSEngine& engine)
{
    /* 捕获 this 指针用于信号发射 */
    auto* self = this;

    /* serialRead() → 返回最近接收的数据(Hex) */
    engine.globalObject().setProperty(
        QStringLiteral("serialRead"),
        engine.evaluate(QStringLiteral("(function() { return _ctx_lastData; })")));

    /* serialWrite(hexString) → 请求发送数据 */
    /* 使用 QJSValue 包装的回调 — 通过 _serialWrite 占位符调用 */
    engine.globalObject().setProperty(
        QStringLiteral("serialWrite"),
        engine.evaluate(QStringLiteral("(function(hexStr) { _serialWriteCallback(hexStr); })")));

    /* log(message) → 输出到控制台 */
    engine.globalObject().setProperty(
        QStringLiteral("log"),
        engine.evaluate(QStringLiteral("(function(msg) { _logCallback(msg); })")));

    /* hexToString(hexStr) → Hex 转文本 */
    engine.globalObject().setProperty(
        QStringLiteral("hexToString"),
        engine.evaluate(QStringLiteral("(function(hex) {"
            " var bytes = hex.replace(/\\s/g,'').match(/.{2}/g);"
            " if(!bytes) return '';"
            " return bytes.map(function(b){ return String.fromCharCode(parseInt(b,16)); }).join('');"
            " })")));

    /* stringToHex(str) → 文本转 Hex */
    engine.globalObject().setProperty(
        QStringLiteral("stringToHex"),
        engine.evaluate(QStringLiteral("(function(str) {"
            " var result = '';"
            " for(var i=0; i<str.length; i++) {"
            "   var hex = str.charCodeAt(i).toString(16).toUpperCase();"
            "   result += (hex.length < 2 ? '0' : '') + hex + ' ';"
            " }"
            " return result.trim();"
            " })")));

    Q_UNUSED(self)
}

/** @brief 替换 ${param} 占位符 */
QString ScriptEngine::substituteParams(const QString& code,
                                       const QMap<QString, QString>& params)
{
    QString result = code;
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        result.replace(QStringLiteral("${%1}").arg(it.key()), it.value());
    }
    return result;
}

// ============================================================
// 编译与缓存
// ============================================================

/** @brief 编译脚本代码(含缓存)，返回 QJSValue(函数或错误) */
QJSValue ScriptEngine::compileScript(QJSEngine& engine, const QString& code)
{
    QString hash = QString::fromUtf8(
        QCryptographicHash::hash(code.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (m_cache.contains(hash)) {
        ++m_cacheHits;
        return m_cache[hash];
    }

    ++m_cacheMisses;
    QJSValue compiled = engine.evaluate(code);

    /* 仅缓存有效的编译结果 */
    if (compiled.isError() || compiled.isUndefined()) {
        return compiled;
    }

    m_cache[hash] = compiled;
    return compiled;
}

// ============================================================
// 核心执行
// ============================================================

/** @brief 执行脚本动作 — 编译→注入上下文→执行→捕获结果 */
ScriptResult ScriptEngine::execute(const ScriptAction& action)
{
    if (!action.enabled) {
        return ScriptResult::fail(tr("脚本 '%1' 已禁用").arg(action.name));
    }

    if (action.code.trimmed().isEmpty()) {
        return ScriptResult::fail(tr("脚本 '%1' 代码为空").arg(action.name));
    }

    if (action.language != ScriptLanguage::JavaScript) {
        return ScriptResult::fail(tr("暂不支持此脚本语言"));
    }

    /* 参数替换 */
    QString code = substituteParams(action.code, action.params);

    QElapsedTimer timer;
    timer.start();

    /* 创建新的 QJSEngine 避免状态污染 */
    QJSEngine engine;

    /* 注入上下文变量 */
    engine.globalObject().setProperty(
        QStringLiteral("_ctx_lastData"),
        QString::fromUtf8(m_lastReceivedData.toHex(' ').toUpper()));

    /* 注入连接信息 */
    QJSValue connInfo = engine.newObject();
    for (auto it = m_connectionInfo.constBegin(); it != m_connectionInfo.constEnd(); ++it) {
        connInfo.setProperty(it.key(), engine.toScriptValue(it.value()));
    }
    engine.globalObject().setProperty(QStringLiteral("connection"), connInfo);

    /* 注入辅助函数 */
    injectHelpers(engine);

    /* 编译执行 */
    QJSValue result = engine.evaluate(code);

    qint64 durationMs = timer.elapsed();

    /* 捕获错误 */
    if (result.isError()) {
        ++m_totalExecutions;
        ++m_totalFailures;
        ScriptResult sr = ScriptResult::fail(
            result.property(QStringLiteral("message")).toString(),
            result.property(QStringLiteral("lineNumber")).toInt(),
            durationMs);
        emit scriptExecuted(sr);
        emit errorOccurred(sr.error);
        return sr;
    }

    /* 成功 */
    ++m_totalExecutions;
    ++m_totalSuccesses;
    m_totalDurationMs += static_cast<quint64>(durationMs);

    QString output;
    /* 获取 console 输出 — 通过全局 _output 捕获 */
    QJSValue outputVal = engine.globalObject().property(QStringLiteral("_output_buffer"));
    if (outputVal.isString()) {
        output = outputVal.toString();
    }

    ScriptResult sr = ScriptResult::ok(output, durationMs, result.toVariant());
    emit scriptExecuted(sr);
    if (!output.isEmpty()) {
        emit outputReady(output);
    }
    return sr;
}

/** @brief 执行裸代码(快捷调试) */
ScriptResult ScriptEngine::executeCode(const QString& code, ScriptLanguage lang)
{
    ScriptAction action;
    action.name = tr("临时脚本");
    action.code = code;
    action.language = lang;
    action.enabled = true;
    return execute(action);
}

// ============================================================
// 上下文绑定
// ============================================================

void ScriptEngine::setLastReceivedData(const QByteArray& data)
{
    m_lastReceivedData = data;
}

void ScriptEngine::setConnectionInfo(const QMap<QString, QVariant>& info)
{
    m_connectionInfo = info;
}

void ScriptEngine::clearContext()
{
    m_lastReceivedData.clear();
    m_connectionInfo.clear();
}

// ============================================================
// 编译缓存
// ============================================================

void ScriptEngine::clearCache() { m_cache.clear(); }
int ScriptEngine::cacheSize() const { return m_cache.size(); }

// ============================================================
// 脚本管理
// ============================================================

QList<ScriptAction> ScriptEngine::scripts() const { return m_scripts; }

void ScriptEngine::setScripts(const QList<ScriptAction>& scripts) { m_scripts = scripts; }

void ScriptEngine::addScript(const ScriptAction& action) { m_scripts.append(action); }

void ScriptEngine::removeScript(int index)
{
    if (index >= 0 && index < m_scripts.size())
        m_scripts.removeAt(index);
}

void ScriptEngine::updateScript(int index, const ScriptAction& action)
{
    if (index >= 0 && index < m_scripts.size())
        m_scripts[index] = action;
}

// 导入/导出和统计见 ScriptEngineStats.cpp

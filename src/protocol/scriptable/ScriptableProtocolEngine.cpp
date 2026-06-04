/**
 * @file ScriptableProtocolEngine.cpp
 * @brief JavaScript 可脚本化协议引擎实现 - 构造/析构/脚本管理/编码解码/JS执行
 *
 * 核心流程:
 *   1. loadScript() 注册脚本(按名称存储到 m_scripts)
 *   2. encode() 将输入数据转 hex → 调用 JS encode(hexStr) → 解析 hex 输出
 *   3. decode() 将输入数据转 hex → 调用 JS decode(hexStr) → 解析 JSON 输出
 *   4. JS 执行错误由 QJSEngine::hasError() 捕获，转为 scriptError 信号
 *   5. 导入/导出使用 JSON 文件格式持久化脚本定义
 *
 * 统计相关方法见 ScriptableProtocolEngineStats.cpp。
 */

#include "protocol/scriptable/ScriptableProtocolEngine.h"

#include <QJSEngine>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDateTime>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造可脚本化协议引擎 @param parent 父对象 */
ScriptableProtocolEngine::ScriptableProtocolEngine(QObject *parent)
    : QObject(parent)
    , m_engine(new QJSEngine(this))
{
}

/** @brief 析构(释放 QJSEngine，QObject 父子树自动管理) */
ScriptableProtocolEngine::~ScriptableProtocolEngine()
{
    m_scripts.clear();
}

// ============================================================================
// 脚本管理
// ============================================================================

/**
 * @brief 加载或更新脚本
 * @param name 脚本唯一名称
 * @param encodeSrc JS 编码脚本源码(须定义 encode(hexStr) 函数)
 * @param decodeSrc JS 解码脚本源码(须定义 decode(hexStr) 函数)
 * @return true=加载成功 false=名称为空
 *
 * 若同名脚本已存在则更新其源码和启用状态。加载后发射 scriptLoaded 信号。
 */
bool ScriptableProtocolEngine::loadScript(const QString &name,
                                          const QString &encodeSrc,
                                          const QString &decodeSrc)
{
    if (name.trimmed().isEmpty()) {
        return false;
    }

    Script script;
    script.name = name;
    script.encodeScript = encodeSrc;
    script.decodeScript = decodeSrc;
    script.enabled = true;

    bool isUpdate = m_scripts.contains(name);
    m_scripts[name] = script;
    ++m_stats.totalScriptsLoaded;

    /* 重新计算活跃脚本数 */
    int activeCount = 0;
    for (auto it = m_scripts.constBegin(); it != m_scripts.constEnd(); ++it) {
        if (it->enabled) {
            ++activeCount;
        }
    }
    m_stats.activeScriptCount = activeCount;

    emit scriptLoaded(name);
    return true;
}

/**
 * @brief 移除脚本
 * @param name 脚本名称
 * @return true=移除成功 false=未找到指定名称
 */
bool ScriptableProtocolEngine::removeScript(const QString &name)
{
    if (!m_scripts.remove(name)) {
        return false;
    }
    ++m_stats.totalScriptsRemoved;

    /* 重新计算活跃脚本数 */
    int activeCount = 0;
    for (auto it = m_scripts.constBegin(); it != m_scripts.constEnd(); ++it) {
        if (it->enabled) {
            ++activeCount;
        }
    }
    m_stats.activeScriptCount = activeCount;

    emit scriptRemoved(name);
    return true;
}

/** @brief 列出所有已加载脚本名称 @return 脚本名称列表 */
QStringList ScriptableProtocolEngine::listScripts() const
{
    return m_scripts.keys();
}

/**
 * @brief 启用/禁用脚本
 * @param name 脚本名称
 * @param enabled true=启用 false=禁用
 */
void ScriptableProtocolEngine::enableScript(const QString &name, bool enabled)
{
    auto it = m_scripts.find(name);
    if (it != m_scripts.end()) {
        it->enabled = enabled;

        /* 重新计算活跃脚本数 */
        int activeCount = 0;
        for (auto iter = m_scripts.constBegin(); iter != m_scripts.constEnd(); ++iter) {
            if (iter->enabled) {
                ++activeCount;
            }
        }
        m_stats.activeScriptCount = activeCount;
    }
}

// ============================================================================
// JS 脚本执行核心
// ============================================================================

/**
 * @brief 在 QJSEngine 中执行指定脚本函数
 * @param source JS 源码(须包含 functionName 对应的函数定义)
 * @param functionName 要调用的函数名("encode" 或 "decode")
 * @param hexArg 十六进制字符串参数
 * @param[out] ok 执行成功标志
 * @return JS 函数返回值(成功时为有效 QJSValue，失败时为 undefined)
 *
 * 执行流程:
 *   1. 在干净的 QJSEngine 中 evaluate 脚本源码
 *   2. 检查语法错误(脚本级错误)
 *   3. 调用指定函数并传入 hexArg
 *   4. 检查运行时错误
 */
QJSValue ScriptableProtocolEngine::evaluateScript(const QString &source,
                                                  const QString &functionName,
                                                  const QString &hexArg,
                                                  bool &ok)
{
    ok = false;

    /* 评估脚本源码(检查语法错误) */
    QJSValue evalResult = m_engine->evaluate(source);
    if (evalResult.isError()) {
        return evalResult;
    }

    /* 查找目标函数 */
    QJSValue func = m_engine->evaluate(functionName);
    if (!func.isCallable()) {
        return QJSValue();
    }

    /* 调用函数 */
    QJSValue result = func.call(QJSValueList() << hexArg);
    if (result.isError()) {
        return result;
    }

    ok = true;
    return result;
}

// ============================================================================
// 编码 / 解码
// ============================================================================

/**
 * @brief 使用指定脚本编码数据
 * @param name 脚本名称
 * @param inputData 待编码的原始数据
 * @return 编码后的字节数据，失败返回空 QByteArray
 *
 * 流程: inputData → hex字符串 → JS encode(hexStr) → hex字符串 → QByteArray
 * JS 脚本须定义 encode(hexStr) 函数，接收十六进制字符串，返回十六进制字符串。
 */
QByteArray ScriptableProtocolEngine::encode(const QString &name,
                                            const QByteArray &inputData)
{
    ++m_stats.totalEncodeCalls;

    auto it = m_scripts.find(name);
    if (it == m_scripts.end() || !it->enabled) {
        emit scriptError(name, tr("脚本不存在或未启用"), true);
        ++m_stats.totalEncodeErrors;
        return QByteArray();
    }

    const Script &script = it.value();
    if (script.encodeScript.trimmed().isEmpty()) {
        emit scriptError(name, tr("编码脚本为空"), true);
        ++m_stats.totalEncodeErrors;
        return QByteArray();
    }

    /* 转换输入为 hex 字符串 */
    QString hexInput = QString::fromUtf8(inputData.toHex());

    /* 执行 JS 编码函数 */
    bool ok = false;
    QJSValue result = evaluateScript(script.encodeScript, "encode", hexInput, ok);

    if (!ok) {
        QString errMsg = result.isError()
            ? tr("JS 运行时错误: %1").arg(result.toString())
            : tr("JS 编码脚本执行失败(函数未定义或语法错误)");
        emit scriptError(name, errMsg, true);
        ++m_stats.totalEncodeErrors;
        return QByteArray();
    }

    /* 解析 hex 字符串输出 */
    QString hexOutput = result.toString();
    QByteArray output = QByteArray::fromHex(hexOutput.toUtf8());

    m_stats.totalEncodeBytes += static_cast<quint64>(output.size());
    emit encodeCompleted(name, output);
    return output;
}

/**
 * @brief 使用指定脚本解码数据
 * @param name 脚本名称
 * @param inputRaw 待解码的原始数据
 * @return 解码后的字段映射(QVariantMap)，失败返回空 QVariantMap
 *
 * 流程: inputRaw → hex字符串 → JS decode(hexStr) → JSON字符串 → QVariantMap
 * JS 脚本须定义 decode(hexStr) 函数，接收十六进制字符串，返回 JSON 对象。
 */
QVariantMap ScriptableProtocolEngine::decode(const QString &name,
                                             const QByteArray &inputRaw)
{
    ++m_stats.totalDecodeCalls;
    m_stats.totalDecodeBytes += static_cast<quint64>(inputRaw.size());

    auto it = m_scripts.find(name);
    if (it == m_scripts.end() || !it->enabled) {
        emit scriptError(name, tr("脚本不存在或未启用"), false);
        ++m_stats.totalDecodeErrors;
        return QVariantMap();
    }

    const Script &script = it.value();
    if (script.decodeScript.trimmed().isEmpty()) {
        emit scriptError(name, tr("解码脚本为空"), false);
        ++m_stats.totalDecodeErrors;
        return QVariantMap();
    }

    /* 转换输入为 hex 字符串 */
    QString hexInput = QString::fromUtf8(inputRaw.toHex());

    /* 执行 JS 解码函数 */
    bool ok = false;
    QJSValue result = evaluateScript(script.decodeScript, "decode", hexInput, ok);

    if (!ok) {
        QString errMsg = result.isError()
            ? tr("JS 运行时错误: %1").arg(result.toString())
            : tr("JS 解码脚本执行失败(函数未定义或语法错误)");
        emit scriptError(name, errMsg, false);
        ++m_stats.totalDecodeErrors;
        return QVariantMap();
    }

    /* 解析 JSON 输出: JS 对象 → JSON.stringify → QJsonDocument → QVariantMap */
    QVariantMap resultMap;
    if (result.isObject()) {
        QJSValue jsonFunc = m_engine->evaluate("JSON.stringify");
        if (jsonFunc.isCallable()) {
            QJSValue jsonStr = jsonFunc.call(QJSValueList() << result);
            if (jsonStr.isString()) {
                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toString().toUtf8());
                if (doc.isObject()) {
                    resultMap = doc.object().toVariantMap();
                }
            }
        }
    } else if (result.isString()) {
        /* 兼容: 脚本直接返回 JSON 字符串 */
        QJsonDocument doc = QJsonDocument::fromJson(result.toString().toUtf8());
        if (doc.isObject()) {
            resultMap = doc.object().toVariantMap();
        }
    }

    emit decodeCompleted(name, resultMap);
    return resultMap;
}

// ============================================================================
// 导入 / 导出
// ============================================================================

/**
 * @brief 导出脚本到 JSON 文件
 * @param name 脚本名称
 * @param path 目标文件路径
 * @return true=导出成功 false=脚本不存在或写入失败
 *
 * JSON 格式:
 * {
 *   "name": "脚本名称",
 *   "encodeScript": "function encode(hex) { ... }",
 *   "decodeScript": "function decode(hex) { ... }",
 *   "enabled": true,
 *   "exportTime": "2026-06-05T12:00:00"
 * }
 */
bool ScriptableProtocolEngine::exportScript(const QString &name,
                                            const QString &path) const
{
    auto it = m_scripts.find(name);
    if (it == m_scripts.constEnd()) {
        return false;
    }

    const Script &script = it.value();
    QJsonObject root;
    root["name"] = script.name;
    root["encodeScript"] = script.encodeScript;
    root["decodeScript"] = script.decodeScript;
    root["enabled"] = script.enabled;
    root["exportTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(root);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/**
 * @brief 从 JSON 文件导入脚本
 * @param path JSON 文件路径
 * @return true=导入成功 false=文件读取/解析失败或名称为空
 *
 * 读取 exportScript() 生成的 JSON 格式，自动注册脚本。
 */
bool ScriptableProtocolEngine::importScript(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QString name = root.value("name").toString();
    if (name.trimmed().isEmpty()) {
        return false;
    }

    QString encodeSrc = root.value("encodeScript").toString();
    QString decodeSrc = root.value("decodeScript").toString();
    bool enabled = root.value("enabled").toBool(true);

    Script script;
    script.name = name;
    script.encodeScript = encodeSrc;
    script.decodeScript = decodeSrc;
    script.enabled = enabled;

    m_scripts[name] = script;
    ++m_stats.totalImports;
    ++m_stats.totalScriptsLoaded;

    /* 重新计算活跃脚本数 */
    int activeCount = 0;
    for (auto it = m_scripts.constBegin(); it != m_scripts.constEnd(); ++it) {
        if (it->enabled) {
            ++activeCount;
        }
    }
    m_stats.activeScriptCount = activeCount;

    emit scriptLoaded(name);
    return true;
}

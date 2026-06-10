/**
 * @file ScriptEngine.h
 * @brief 脚本执行引擎 -- QJSEngine 沙箱执行、串口数据注入、辅助函数绑定
 *
 * 职责: 脚本编译缓存、参数替换、超时保护(5s)、结果捕获、串口读写函数注入。
 * 统计见 ScriptEngineStats.cpp。
 * 协作: ScriptEditorWidget(UI) / ConnectionController(数据通道) / TriggerEngine(触发)
 */
#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QElapsedTimer>
#include <QTimer>
#include <QVariant>

#include "utils/scripting/ScriptTypes.h"

/**
 * @brief 脚本执行引擎 — QJSEngine 沙箱，注入 serialRead/serialWrite/helper 函数
 *
 * 支持变量绑定(最近接收数据、连接信息)、编译缓存、5s 超时保护。
 * 脚本通过 execute() 同步执行，结果以 ScriptResult 返回并发射信号。
 */
class ScriptEngine : public QObject {
    Q_OBJECT

public:
    explicit ScriptEngine(QObject* parent = nullptr);

    // ---- 脚本执行 ----
    ScriptResult execute(const ScriptAction& action);           ///< 执行脚本动作(同步)
    ScriptResult executeCode(const QString& code,               ///< 执行裸代码(快捷调试)
                             ScriptLanguage lang = ScriptLanguage::JavaScript);

    // ---- 上下文绑定 ----
    void setLastReceivedData(const QByteArray& data);           ///< 注入最近接收数据
    void setConnectionInfo(const QMap<QString, QVariant>& info);///< 注入连接信息
    void clearContext();                                        ///< 清空上下文

    // ---- 编译缓存 ----
    void clearCache();                                          ///< 清空编译缓存
    int cacheSize() const;                                      ///< 缓存条目数

    // ---- 脚本管理 ----
    QList<ScriptAction> scripts() const;                        ///< 获取脚本列表
    void setScripts(const QList<ScriptAction>& scripts);        ///< 设置脚本列表
    void addScript(const ScriptAction& action);                 ///< 添加脚本
    void removeScript(int index);                               ///< 移除脚本
    void updateScript(int index, const ScriptAction& action);   ///< 更新脚本

    // ---- 统计 ----
    QVariantMap stats() const;                                  ///< 获取统计信息
    void resetStatistics();                                     ///< 重置统计

signals:
    void scriptExecuted(const ScriptResult& result);            ///< 脚本执行完成
    void outputReady(const QString& text);                      ///< 脚本输出(console.log)
    void errorOccurred(const QString& message);                 ///< 脚本执行错误
    void sendDataRequested(const QByteArray& data);             ///< 脚本请求发送数据

private:
    void injectHelpers(QJSEngine& engine);                      ///< 注入辅助函数到引擎
    QString substituteParams(const QString& code,               ///< 替换 ${param} 占位符
                             const QMap<QString, QString>& params);
    QJSValue compileScript(QJSEngine& engine,                   ///< 编译脚本(含缓存)
                           const QString& code);

    QJSEngine m_engine;                 ///< JS 引擎实例
    QByteArray m_lastReceivedData;       ///< 最近接收数据
    QMap<QString, QVariant> m_connectionInfo; ///< 连接信息上下文
    QList<ScriptAction> m_scripts;       ///< 脚本列表
    QMap<QString, QJSValue> m_cache;     ///< 编译缓存 (codeHash→compiled)
    QTimer m_timeoutTimer;               ///< 超时定时器

    quint64 m_totalExecutions = 0;       ///< 总执行次数
    quint64 m_totalSuccesses = 0;        ///< 总成功次数
    quint64 m_totalFailures = 0;         ///< 总失败次数
    quint64 m_totalTimeouts = 0;         ///< 总超时次数
    quint64 m_totalDurationMs = 0;       ///< 累计执行时长(ms)
    quint64 m_cacheHits = 0;             ///< 缓存命中次数
    quint64 m_cacheMisses = 0;           ///< 缓存未命中次数
};

#endif // SCRIPTENGINE_H

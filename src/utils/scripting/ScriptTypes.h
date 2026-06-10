/**
 * @file ScriptTypes.h
 * @brief Script Engine 类型定义 -- 脚本语言枚举、动作配置、执行结果
 *
 * 被 ScriptEngine 和 ScriptEditorWidget 共用。所有脚本元数据和结果类型集中定义。
 */
#ifndef SCRIPTTYPES_H
#define SCRIPTTYPES_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>

/** @brief 支持的脚本语言 */
enum class ScriptLanguage {
    JavaScript,     ///< QJSEngine (ECMAScript)
    Python          ///< 预留: 嵌入式 Python (暂未实现)
};

/** @brief 脚本动作配置 — 一个可保存/加载的脚本单元 */
struct ScriptAction {
    QString name;                           ///< 脚本显示名称
    QString code;                           ///< 脚本源码
    ScriptLanguage language = ScriptLanguage::JavaScript; ///< 脚本语言
    QStringList triggers;                   ///< 触发条件列表 (onDataReceived / onConnected / manual)
    QMap<QString, QString> params;          ///< 用户可配置参数 (key→defaultValue)
    bool enabled = true;                    ///< 是否启用

    /** @brief 从 QVariantMap 反序列化 */
    static ScriptAction fromVariantMap(const QMap<QString, QVariant>& map);
    /** @brief 序列化为 QVariantMap */
    QMap<QString, QVariant> toVariantMap() const;
};

/** @brief 脚本执行结果 — 单次执行的完整输出 */
struct ScriptResult {
    bool success = false;           ///< 执行是否成功
    QString output;                 ///< 标准输出 (console.log / print)
    QString error;                  ///< 错误信息 (语法错误 / 运行时异常)
    int line = -1;                  ///< 错误行号 (-1 表示未知)
    qint64 durationMs = 0;          ///< 执行耗时 (毫秒)
    QVariant returnValue;           ///< 脚本返回值 (eval 最后表达式的值)

    /** @brief 创建一个成功结果 */
    static ScriptResult ok(const QString& output, qint64 durationMs, const QVariant& ret = {});
    /** @brief 创建一个失败结果 */
    static ScriptResult fail(const QString& error, int line = -1, qint64 durationMs = 0);
};

#endif // SCRIPTTYPES_H

/**
 * @file ScriptableProtocolEngine.h
 * @brief JavaScript 可脚本化协议引擎 -- 用户通过 JS 脚本自定义协议编码/解码逻辑
 *
 * 核心设计:
 *   - 用户编写 JS 脚本定义 encode(inputHex)→outputHex 和 decode(inputHex)→JSON
 *   - 引擎使用 QJSEngine 执行脚本，输入输出均以十六进制字符串形式传递
 *   - 支持脚本加载/移除/导入/导出，多脚本并存按名称管理
 *   - 统计模块记录编码解码调用次数、错误率、累计字节数
 *
 * 协作: ProtocolEngine(静态schema引擎)/自动化触发器/发送控制器
 */

#ifndef SCRIPTABLE_PROTOCOL_ENGINE_H
#define SCRIPTABLE_PROTOCOL_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QVariantMap>
#include <QJSValue>

class QJSEngine;

/**
 * @brief JavaScript 可脚本化协议引擎
 *
 * 用户通过 loadScript() 注册 JS 编码/解码脚本，通过 encode()/decode()
 * 调用指定脚本处理数据。脚本以十六进制字符串作为 I/O 格式，降低二进制
 * 处理门槛。JS 执行错误由引擎捕获并转为信号通知，不抛出 C++ 异常。
 */
class ScriptableProtocolEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 脚本定义 */
    struct Script {
        QString name;           ///< 脚本唯一名称(用户指定，用作索引键)
        QString encodeScript;   ///< JS 编码脚本源码(须定义 encode(hexStr) 函数)
        QString decodeScript;   ///< JS 解码脚本源码(须定义 decode(hexStr) 函数)
        bool enabled = true;    ///< 脚本是否启用
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodeCalls = 0;       ///< 编码调用总次数
        quint64 totalDecodeCalls = 0;       ///< 解码调用总次数
        quint64 totalEncodeErrors = 0;      ///< 编码 JS 执行错误次数
        quint64 totalDecodeErrors = 0;      ///< 解码 JS 执行错误次数
        quint64 totalEncodeBytes = 0;       ///< 编码输出累计字节数
        quint64 totalDecodeBytes = 0;       ///< 解码输入累计字节数
        quint64 totalScriptsLoaded = 0;     ///< 累计加载脚本次数
        quint64 totalScriptsRemoved = 0;    ///< 累计移除脚本次数
        quint64 totalImports = 0;           ///< 累计导入脚本次数
        quint64 totalExports = 0;           ///< 累计导出脚本次数
        int activeScriptCount = 0;          ///< 当前活跃(已启用)脚本数
    };

    //-- 构造/析构 --//
    explicit ScriptableProtocolEngine(QObject *parent = nullptr); ///< 构造(初始化QJSEngine)
    ~ScriptableProtocolEngine() override;                         ///< 析构

    //-- 脚本管理 --//
    bool loadScript(const QString &name, const QString &encodeSrc,
                    const QString &decodeSrc);  ///< 加载/更新脚本(按名称索引)
    bool removeScript(const QString &name);     ///< 移除脚本(按名称)
    QStringList listScripts() const;            ///< 列出所有已加载脚本名称
    void enableScript(const QString &name, bool enabled); ///< 启用/禁用脚本

    //-- 编码/解码 --//
    QByteArray encode(const QString &name, const QByteArray &inputData); ///< 编码: 输入→JS→hex输出
    QVariantMap decode(const QString &name, const QByteArray &inputRaw); ///< 解码: raw hex→JS→JSON

    //-- 导入/导出 --//
    bool exportScript(const QString &name, const QString &path) const; ///< 导出脚本到JSON文件
    bool importScript(const QString &path);                            ///< 从JSON文件导入脚本

    //-- 统计 --//
    const Stats &stats() const;  ///< 获取运行统计(const引用)
    void resetStatistics();      ///< 重置所有统计计数器

signals:
    /** @brief 编码完成 @param name 脚本名称 @param output 编码后的数据 */
    void encodeCompleted(const QString &name, const QByteArray &output);
    /** @brief 解码完成 @param name 脚本名称 @param result 解码结果 */
    void decodeCompleted(const QString &name, const QVariantMap &result);
    /** @brief 脚本执行错误 @param name 脚本名称 @param error 错误描述 @param isEncode true=编码阶段 false=解码阶段 */
    void scriptError(const QString &name, const QString &error, bool isEncode);
    /** @brief 脚本加载完成 @param name 脚本名称 */
    void scriptLoaded(const QString &name);
    /** @brief 脚本移除 @param name 脚本名称 */
    void scriptRemoved(const QString &name);

private:
    /** @brief 在 QJSEngine 中执行指定脚本函数 @param source JS 源码 @param functionName 函数名("encode"/"decode") @param hexArg 十六进制字符串参数 @param[out] ok 执行成功标志 @return 函数返回值 */
    QJSValue evaluateScript(const QString &source, const QString &functionName,
                            const QString &hexArg, bool &ok);

    QJSEngine *m_engine = nullptr;          ///< JS 执行引擎
    QMap<QString, Script> m_scripts;        ///< 已加载脚本(按名称索引)
    Stats m_stats;                          ///< 运行统计
};

#endif // SCRIPTABLE_PROTOCOL_ENGINE_H

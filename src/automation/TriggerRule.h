/**
 * @file TriggerRule.h
 * @brief 触发器规则数据结构 — 定义自动化触发匹配和动作配置
 *
 * 提供触发器规则的配置结构体和序列化/反序列化方法。
 * 纯 C++ 头文件，无 Q_OBJECT，无 QObject 继承。
 *
 * 协作关系:
 *   - TriggerEngine: 使用规则进行数据匹配评估
 *   - TriggerAction: 根据规则动作类型执行操作
 *   - TriggerManager: 管理规则的持久化和生命周期
 *   - TriggerListPanel: UI 展示和编辑规则
 */
#ifndef TRIGGERRULE_H
#define TRIGGERRULE_H

#include <QString>
#include <QByteArray>
#include <QCoreApplication>
#include <QJsonObject>

/**
 * @brief 匹配模式枚举
 * 定义数据匹配的不同方式
 */
enum class MatchMode {
    ExactString,    ///< 精确字符串匹配
    Regex,          ///< 正则表达式匹配
    HexBytes,       ///< 十六进制字节序列匹配
    ValueRange      ///< 数值范围匹配（需配合 valueMin/valueMax）
};

/**
 * @brief 动作类型枚举
 * 定义触发器命中后执行的动作类型
 */
enum class ActionType {
    SendData,           ///< 发送预设数据
    StartRecording,     ///< 开始录制
    StopRecording,      ///< 停止录制
    ShowToast,          ///< 显示提示消息
    PlaySound           ///< 播放提示音
};

/**
 * @brief 触发器规则配置结构体
 *
 * 包含规则名称、匹配模式、匹配模式、动作类型和动作数据等完整配置。
 * 支持通过 JSON 进行序列化和反序列化。
 */
struct TriggerRuleConfig {
    QString name;               ///< 规则名称
    MatchMode matchMode;        ///< 匹配模式
    QString pattern;            ///< 匹配模式字符串（正则/精确匹配/十六进制）
    double valueMin = 0;        ///< 数值范围下界（ValueRange 模式使用）
    double valueMax = 0;        ///< 数值范围上界（ValueRange 模式使用）
    ActionType actionType;      ///< 动作类型
    QByteArray actionData;      ///< 动作附加数据（如发送的内容）
    bool enabled = true;        ///< 是否启用

    /**
     * @brief 创建默认规则配置
     * @return 包含合理默认值的规则配置
     */
    static TriggerRuleConfig createDefault()
    {
        TriggerRuleConfig config;
        config.name = QCoreApplication::translate("TriggerRule", "新规则");
        config.matchMode = MatchMode::ExactString;
        config.pattern = QString();
        config.valueMin = 0;
        config.valueMax = 0;
        config.actionType = ActionType::SendData;
        config.actionData = QByteArray();
        config.enabled = true;
        return config;
    }

    /**
     * @brief 从 JSON 对象反序列化规则配置
     * @param json JSON 对象
     * @return 解析后的规则配置
     */
    static TriggerRuleConfig fromJson(const QJsonObject& json)
    {
        TriggerRuleConfig config;
        config.name = json["name"].toString(QStringLiteral("未命名规则"));
        config.matchMode = static_cast<MatchMode>(json["matchMode"].toInt(0));
        config.pattern = json["pattern"].toString();
        config.valueMin = json["valueMin"].toDouble(0.0);
        config.valueMax = json["valueMax"].toDouble(0.0);
        config.actionType = static_cast<ActionType>(json["actionType"].toInt(0));
        config.actionData = QByteArray::fromBase64(json["actionData"].toString().toUtf8());
        config.enabled = json["enabled"].toBool(true);
        return config;
    }

    /**
     * @brief 将规则配置序列化为 JSON 对象
     * @param rule 规则配置
     * @return JSON 对象
     */
    static QJsonObject toJson(const TriggerRuleConfig& rule)
    {
        QJsonObject obj;
        obj["name"] = rule.name;
        obj["matchMode"] = static_cast<int>(rule.matchMode);
        obj["pattern"] = rule.pattern;
        obj["valueMin"] = rule.valueMin;
        obj["valueMax"] = rule.valueMax;
        obj["actionType"] = static_cast<int>(rule.actionType);
        obj["actionData"] = QString::fromUtf8(rule.actionData.toBase64());
        obj["enabled"] = rule.enabled;
        return obj;
    }
};

#endif // TRIGGERRULE_H

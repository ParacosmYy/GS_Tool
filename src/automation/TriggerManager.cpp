/**
 * @file TriggerManager.cpp
 * @brief 触发器管理器实现 — 门面模式，协调引擎和动作执行器
 */

#include "automation/TriggerManager.h"
#include "automation/TriggerEngine.h"
#include "automation/TriggerAction.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

/**
 * @brief 构造函数
 *
 * 创建 TriggerEngine 和 TriggerAction 实例并建立信号连接:
 *   engine::actionRequired → action::execute
 *
 * @param parent 父对象
 */
TriggerManager::TriggerManager(QObject* parent)
    : QObject(parent)
    , m_engine(new TriggerEngine(this))
    , m_action(new TriggerAction(this))
{
    /* 引擎匹配命中后，转发到动作执行器 */
    connect(m_engine, &TriggerEngine::actionRequired,
            m_action, &TriggerAction::execute);
}

/** @brief 析构函数 */
TriggerManager::~TriggerManager()
{
    // QObject 父子树自动销毁 m_engine 和 m_action
}

/**
 * @brief 从文件加载规则
 *
 * 读取 JSON 文件，解析为规则列表，同步到引擎。
 * JSON 格式为包含 TriggerRuleConfig 对象的数组。
 *
 * @param filePath JSON 文件路径
 * @return true 加载成功，false 加载失败
 */
bool TriggerManager::loadRules(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (doc.isNull()) {
        return false;
    }

    if (!doc.isArray()) {
        return false;
    }

    /* 先清空引擎中的旧规则 */
    const QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            TriggerRuleConfig rule = TriggerRuleConfig::fromJson(val.toObject());
            m_engine->addRule(rule);
        }
    }

    emit rulesChanged();
    return true;
}

/**
 * @brief 保存规则到文件
 *
 * 将当前引擎中的所有规则序列化为 JSON 数组写入文件。
 *
 * @param filePath JSON 文件路径
 * @return true 保存成功，false 保存失败
 */
bool TriggerManager::saveRules(const QString& filePath)
{
    QJsonArray arr;
    const auto& ruleList = m_engine->rules();
    for (const TriggerRuleConfig& rule : ruleList) {
        arr.append(TriggerRuleConfig::toJson(rule));
    }

    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/**
 * @brief 获取所有规则
 * @return 规则配置列表
 */
QList<TriggerRuleConfig> TriggerManager::rules() const
{
    return m_engine->rules();
}

/**
 * @brief 添加一条规则
 * @param rule 规则配置
 */
void TriggerManager::addRule(const TriggerRuleConfig& rule)
{
    m_engine->addRule(rule);
    emit rulesChanged();
}

/**
 * @brief 移除指定索引的规则
 * @param index 规则索引
 */
void TriggerManager::removeRule(int index)
{
    m_engine->removeRule(index);
    emit rulesChanged();
}

/**
 * @brief 设置指定规则的启用/禁用状态
 * @param index 规则索引
 * @param enabled true 启用，false 禁用
 */
void TriggerManager::setRuleEnabled(int index, bool enabled)
{
    m_engine->setRuleEnabled(index, enabled);
}

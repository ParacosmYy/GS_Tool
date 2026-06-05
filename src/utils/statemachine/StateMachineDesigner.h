/**
 * @file StateMachineDesigner.h
 * @brief 状态机设计器 — 状态机定义、验证与代码生成引擎
 * @author Serial Tool Team
 * @date 2026-06-06
 *
 * 提供状态机的 CRUD 管理、合法性验证（不可达状态/死端/重复/缺初始状态）、
 * 以及多格式导出（C/Python/JSON/Graphviz DOT）。
 */

#ifndef STATEMACHINEDESIGNER_H
#define STATEMACHINEDESIGNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

#include "utils/statemachine/SmTypes.h"

/**
 * @class StateMachineDesigner
 * @brief 状态机定义和验证引擎
 *
 * 管理状态与迁移的 CRUD 操作，执行合法性校验，
 * 并支持导出为 C switch-case、Python 字典、JSON、Graphviz DOT 格式。
 */
class StateMachineDesigner : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造状态机设计器 @param parent 父对象 */
    explicit StateMachineDesigner(QObject *parent = nullptr);

    /** @brief 设置完整状态机模型 @param machine 状态机数据 */
    void setMachine(const SmMachine &machine);

    /** @brief 获取当前状态机模型 @return 状态机数据 */
    SmMachine machine() const;

    // ---- 状态管理 ----

    /** @brief 添加状态 @param state 状态数据 @return 成功返回true，名称重复返回false */
    bool addState(const SmState &state);

    /** @brief 移除状态 @param name 状态名称 @return 成功返回true */
    bool removeState(const QString &name);

    /** @brief 更新状态 @param name 原状态名称 @param state 新状态数据 @return 成功返回true */
    bool updateState(const QString &name, const SmState &state);

    /** @brief 按名称查询状态 @param name 状态名称 @return 状态数据，不存在则返回默认值 */
    SmState state(const QString &name) const;

    /** @brief 获取全部状态 @return 状态列表 */
    QVector<SmState> states() const;

    // ---- 迁移管理 ----

    /** @brief 添加迁移 @param trans 迁移数据 @return 成功返回true */
    bool addTransition(const SmTransition &trans);

    /** @brief 移除迁移 @param index 迁移索引 @return 成功返回true */
    bool removeTransition(int index);

    /** @brief 更新迁移 @param index 迁移索引 @param trans 新迁移数据 @return 成功返回true */
    bool updateTransition(int index, const SmTransition &trans);

    /** @brief 获取全部迁移 @return 迁移列表 */
    QVector<SmTransition> transitions() const;

    // ---- 验证 ----

    /** @brief 验证状态机合法性 @return 全部通过返回true */
    bool validate() const;

    /** @brief 获取验证错误列表 @return 错误描述字符串列表 */
    QStringList validationErrors() const;

    // ---- 导出 ----

    /** @brief 导出为指定格式字符串 @param format 目标格式 @return 格式化内容 */
    QString exportToFormat(SmExportFormat format) const;

    /** @brief 导出到文件 @param filePath 文件路径 @param format 目标格式 @return 成功返回true */
    bool exportToFile(const QString &filePath, SmExportFormat format) const;

    // ---- 统计 ----
    quint64 totalStatesAdded() const;
    quint64 totalStatesRemoved() const;
    quint64 totalTransitionsAdded() const;
    quint64 totalTransitionsRemoved() const;
    quint64 totalValidations() const;
    quint64 totalExports() const;
    void resetStatistics();

signals:
    /** @brief 状态机内容变更信号 */
    void machineChanged();
    /** @brief 状态已添加 @param name 状态名称 */
    void stateAdded(const QString &name);
    /** @brief 状态已移除 @param name 状态名称 */
    void stateRemoved(const QString &name);
    /** @brief 迁移已添加 @param index 迁移索引 */
    void transitionAdded(int index);

private:
    QString exportToC() const;
    QString exportToPython() const;
    QString exportToJson() const;
    QString exportToGraphviz() const;

    SmMachine m_machine;                    ///< 当前状态机模型
    mutable QStringList m_validationErrors; ///< 缓存的验证错误
    mutable quint64 m_totalStatesAdded = 0;
    mutable quint64 m_totalStatesRemoved = 0;
    mutable quint64 m_totalTransitionsAdded = 0;
    mutable quint64 m_totalTransitionsRemoved = 0;
    mutable quint64 m_totalValidations = 0;
    mutable quint64 m_totalExports = 0;
};

#endif // STATEMACHINEDESIGNER_H

/**
 * @file SmTypes.h
 * @brief 状态机数据类型定义
 * @author Serial Tool Team
 * @date 2026-06-06
 *
 * 定义状态机设计器所需的全部数据结构，包括状态、迁移和状态机模型，
 * 以及导出格式枚举。
 */

#ifndef SMTYPES_H
#define SMTYPES_H

#include <QPointF>
#include <QString>
#include <QVector>

/**
 * @brief 状态机中的单个状态
 */
struct SmState {
    QString name;           ///< 状态名称（唯一标识符）
    QString entryAction;    ///< 进入动作（C/Python 代码片段）
    QString exitAction;     ///< 退出动作（C/Python 代码片段）
    bool isInitial = false; ///< 是否为初始状态
    bool isFinal = false;   ///< 是否为终止状态
    QPointF position;       ///< 画布坐标位置
};

/**
 * @brief 状态之间的迁移（转换）
 */
struct SmTransition {
    QString sourceState;    ///< 源状态名称
    QString targetState;    ///< 目标状态名称
    QString event;          ///< 触发事件名称
    QString guard;          ///< 守卫条件（布尔表达式）
    QString action;         ///< 迁移时执行的动作
};

/**
 * @brief 完整的状态机模型
 */
struct SmMachine {
    QString name;                   ///< 状态机名称
    QVector<SmState> states;        ///< 状态集合
    QVector<SmTransition> transitions; ///< 迁移集合
    QString initialState;           ///< 初始状态名称
};

/**
 * @brief 导出格式枚举
 */
enum class SmExportFormat {
    C,          ///< C 语言 switch-case 模式
    Python,     ///< Python 字典/函数模式
    Json,       ///< JSON 序列化
    Graphviz    ///< Graphviz DOT 格式
};

#endif // SMTYPES_H

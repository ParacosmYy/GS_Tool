/**
 * @file StateMachineDesigner.cpp
 * @brief 状态机设计器实现 — CRUD / 验证 / 导出
 * @author Serial Tool Team
 * @date 2026-06-06
 */

#include "utils/statemachine/StateMachineDesigner.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

/** @brief 构造函数 @param parent 父对象 */
StateMachineDesigner::StateMachineDesigner(QObject *parent)
    : QObject(parent)
{
}

/** @brief 设置完整状态机模型并发射变更信号 @param machine 状态机数据 */
void StateMachineDesigner::setMachine(const SmMachine &machine)
{
    m_machine = machine;
    emit machineChanged();
}

/** @brief 获取当前状态机模型 @return 状态机数据 */
SmMachine StateMachineDesigner::machine() const
{
    return m_machine;
}

// ---- 状态 CRUD ----

/** @brief 添加状态（名称唯一性检查） @param state 状态数据 @return 成功返回true */
bool StateMachineDesigner::addState(const SmState &state)
{
    for (const auto &s : m_machine.states) {
        if (s.name == state.name) return false;
    }
    m_machine.states.append(state);
    ++m_totalStatesAdded;
    if (state.isInitial) {
        m_machine.initialState = state.name;
    }
    emit stateAdded(state.name);
    emit machineChanged();
    return true;
}

/** @brief 移除状态及其关联迁移 @param name 状态名称 @return 成功返回true */
bool StateMachineDesigner::removeState(const QString &name)
{
    for (int i = 0; i < m_machine.states.size(); ++i) {
        if (m_machine.states.at(i).name == name) {
            m_machine.states.removeAt(i);
            /* 移除关联迁移 */
            for (int j = m_machine.transitions.size() - 1; j >= 0; --j) {
                const auto &t = m_machine.transitions.at(j);
                if (t.sourceState == name || t.targetState == name) {
                    m_machine.transitions.removeAt(j);
                }
            }
            if (m_machine.initialState == name) {
                m_machine.initialState.clear();
            }
            ++m_totalStatesRemoved;
            emit stateRemoved(name);
            emit machineChanged();
            return true;
        }
    }
    return false;
}

/** @brief 更新状态 @param name 原状态名称 @param state 新数据 @return 成功返回true */
bool StateMachineDesigner::updateState(const QString &name, const SmState &state)
{
    for (int i = 0; i < m_machine.states.size(); ++i) {
        if (m_machine.states.at(i).name == name) {
            m_machine.states[i] = state;
            if (state.isInitial) {
                m_machine.initialState = state.name;
            }
            emit machineChanged();
            return true;
        }
    }
    return false;
}

/** @brief 按名称查询状态 @param name 状态名称 @return 状态数据 */
SmState StateMachineDesigner::state(const QString &name) const
{
    for (const auto &s : m_machine.states) {
        if (s.name == name) return s;
    }
    return SmState{};
}

/** @brief 获取全部状态 @return 状态列表 */
QVector<SmState> StateMachineDesigner::states() const
{
    return m_machine.states;
}

// ---- 迁移 CRUD ----

/** @brief 添加迁移 @param trans 迁移数据 @return 成功返回true */
bool StateMachineDesigner::addTransition(const SmTransition &trans)
{
    m_machine.transitions.append(trans);
    ++m_totalTransitionsAdded;
    emit transitionAdded(m_machine.transitions.size() - 1);
    emit machineChanged();
    return true;
}

/** @brief 移除迁移 @param index 迁移索引 @return 成功返回true */
bool StateMachineDesigner::removeTransition(int index)
{
    if (index < 0 || index >= m_machine.transitions.size()) return false;
    m_machine.transitions.removeAt(index);
    ++m_totalTransitionsRemoved;
    emit machineChanged();
    return true;
}

/** @brief 更新迁移 @param index 迁移索引 @param trans 新数据 @return 成功返回true */
bool StateMachineDesigner::updateTransition(int index, const SmTransition &trans)
{
    if (index < 0 || index >= m_machine.transitions.size()) return false;
    m_machine.transitions[index] = trans;
    emit machineChanged();
    return true;
}

/** @brief 获取全部迁移 @return 迁移列表 */
QVector<SmTransition> StateMachineDesigner::transitions() const
{
    return m_machine.transitions;
}

// ---- 验证 ----

/** @brief 执行完整验证并缓存结果 @return 全部通过返回true */
bool StateMachineDesigner::validate() const
{
    m_validationErrors.clear();
    ++m_totalValidations;

    if (m_machine.name.isEmpty()) {
        m_validationErrors.append(tr("状态机名称不能为空"));
    }

    /* 检查空状态集 */
    if (m_machine.states.isEmpty()) {
        m_validationErrors.append(tr("状态机没有定义任何状态"));
        return false;
    }

    /* 重复名称检查 */
    QSet<QString> names;
    for (const auto &s : m_machine.states) {
        if (names.contains(s.name)) {
            m_validationErrors.append(tr("重复的状态名称: %1").arg(s.name));
        }
        names.insert(s.name);
        if (s.name.isEmpty()) {
            m_validationErrors.append(tr("存在未命名的状态"));
        }
    }

    /* 初始状态检查 */
    bool hasInitial = false;
    for (const auto &s : m_machine.states) {
        if (s.isInitial) { hasInitial = true; break; }
    }
    if (!hasInitial) {
        m_validationErrors.append(tr("缺少初始状态"));
    }

    /* 不可达状态检查（BFS 从初始状态出发） */
    QString initName;
    for (const auto &s : m_machine.states) {
        if (s.isInitial) { initName = s.name; break; }
    }
    if (!initName.isEmpty()) {
        QSet<QString> reachable;
        QVector<QString> queue;
        queue.append(initName);
        reachable.insert(initName);
        while (!queue.isEmpty()) {
            QString cur = queue.takeFirst();
            for (const auto &t : m_machine.transitions) {
                if (t.sourceState == cur && !reachable.contains(t.targetState)) {
                    reachable.insert(t.targetState);
                    queue.append(t.targetState);
                }
            }
        }
        for (const auto &s : m_machine.states) {
            if (!reachable.contains(s.name)) {
                m_validationErrors.append(
                    tr("不可达状态: %1").arg(s.name));
            }
        }
    }

    /* 死端检查（非终态无出迁移） */
    for (const auto &s : m_machine.states) {
        if (s.isFinal) continue;
        bool hasOut = false;
        for (const auto &t : m_machine.transitions) {
            if (t.sourceState == s.name) { hasOut = true; break; }
        }
        if (!hasOut) {
            m_validationErrors.append(
                tr("死端状态（无出迁移）: %1").arg(s.name));
        }
    }

    /* 迁移引用检查 */
    for (const auto &t : m_machine.transitions) {
        if (!names.contains(t.sourceState)) {
            m_validationErrors.append(
                tr("迁移引用不存在的源状态: %1").arg(t.sourceState));
        }
        if (!names.contains(t.targetState)) {
            m_validationErrors.append(
                tr("迁移引用不存在的目标状态: %1").arg(t.targetState));
        }
    }

    return m_validationErrors.isEmpty();
}

/** @brief 获取最近一次验证的错误列表 @return 错误描述列表 */
QStringList StateMachineDesigner::validationErrors() const
{
    return m_validationErrors;
}

// ---- 导出 ----

/** @brief 按格式导出为字符串 @param format 目标格式 @return 格式化内容 */
QString StateMachineDesigner::exportToFormat(SmExportFormat format) const
{
    ++m_totalExports;
    switch (format) {
    case SmExportFormat::C:        return exportToC();
    case SmExportFormat::Python:   return exportToPython();
    case SmExportFormat::Json:     return exportToJson();
    case SmExportFormat::Graphviz: return exportToGraphviz();
    }
    return QString();
}

/** @brief 导出到文件 @param filePath 路径 @param format 格式 @return 成功返回true */
bool StateMachineDesigner::exportToFile(const QString &filePath,
                                         SmExportFormat format) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out << exportToFormat(format);
    file.close();
    return true;
}

/** @brief 导出C语言 switch-case 状态机模式 @return C代码字符串 */
QString StateMachineDesigner::exportToC() const
{
    QString code;
    QTextStream s(&code);
    QString guard = m_machine.name.toUpper();
    if (guard.isEmpty()) guard = QStringLiteral("SM");

    s << "/* Auto-generated state machine: " << m_machine.name << " */\n";
    s << "#ifndef " << guard << "_H\n";
    s << "#define " << guard << "_H\n\n";
    s << "#include <stdint.h>\n\n";
    s << "typedef enum {\n";
    for (int i = 0; i < m_machine.states.size(); ++i) {
        s << "    " << m_machine.states.at(i).name;
        if (i + 1 < m_machine.states.size()) s << QStringLiteral(",");
        s << QStringLiteral("\n");
    }
    s << "} " << m_machine.name << "_State;\n\n";
    s << "void " << m_machine.name << "_run(" << m_machine.name
      << "_State *state, int event) {\n";
    s << "    switch (*state) {\n";
    for (const auto &st : m_machine.states) {
        s << "    case " << st.name << ":\n";
        if (!st.entryAction.isEmpty()) {
            s << "        /* entry: " << st.entryAction << " */\n";
        }
        bool hasTrans = false;
        for (const auto &t : m_machine.transitions) {
            if (t.sourceState == st.name) {
                hasTrans = true;
                s << "        if (event == " << t.event;
                if (!t.guard.isEmpty()) {
                    s << " && (" << t.guard << ")";
                }
                s << ") {\n";
                if (!t.action.isEmpty()) {
                    s << "            " << t.action << ";\n";
                }
                s << "            *state = " << t.targetState << ";\n";
                s << "            return;\n        }\n";
            }
        }
        if (!hasTrans) {
            s << "        /* no outgoing transitions */\n";
        }
        s << "        break;\n";
    }
    s << "    }\n}\n\n#endif /* " << guard << "_H */\n";
    return code;
}

/** @brief 导出Python字典模式 @return Python代码字符串 */
QString StateMachineDesigner::exportToPython() const
{
    QString code;
    QTextStream s(&code);
    s << "# Auto-generated state machine: " << m_machine.name << "\n\n";
    s << "class " << m_machine.name << ":\n";
    s << "    def __init__(self):\n";
    s << "        self.state = \"" << m_machine.initialState << "\"\n";
    s << "        self.transitions = {\n";
    for (int i = 0; i < m_machine.transitions.size(); ++i) {
        const auto &t = m_machine.transitions.at(i);
        s << "        (\"" << t.sourceState << "\", \"" << t.event << "\"): \""
          << t.targetState << "\"";
        if (i + 1 < m_machine.transitions.size()) s << QStringLiteral(",");
        s << QStringLiteral("\n");
    }
    s << "    }\n\n";
    s << "    def handle(self, event):\n";
    s << "        key = (self.state, event)\n";
    s << "        if key in self.transitions:\n";
    s << "            self.state = self.transitions[key]\n";
    s << "            return True\n";
    s << "        return False\n\n";
    for (const auto &st : m_machine.states) {
        if (!st.entryAction.isEmpty() || !st.exitAction.isEmpty()) {
            s << "    def _on_" << st.name << "(self):\n";
            if (!st.entryAction.isEmpty()) {
                s << "        # entry: " << st.entryAction << "\n";
            }
            if (!st.exitAction.isEmpty()) {
                s << "        # exit: " << st.exitAction << "\n";
            }
            s << QStringLiteral("\n");
        }
    }
    return code;
}

/** @brief 导出JSON序列化 @return JSON字符串 */
QString StateMachineDesigner::exportToJson() const
{
    QJsonObject root;
    root[QStringLiteral("name")] = m_machine.name;
    root[QStringLiteral("initialState")] = m_machine.initialState;
    QJsonArray statesArr;
    for (const auto &st : m_machine.states) {
        QJsonObject obj;
        obj[QStringLiteral("name")] = st.name;
        obj[QStringLiteral("entryAction")] = st.entryAction;
        obj[QStringLiteral("exitAction")] = st.exitAction;
        obj[QStringLiteral("isInitial")] = st.isInitial;
        obj[QStringLiteral("isFinal")] = st.isFinal;
        obj[QStringLiteral("x")] = st.position.x();
        obj[QStringLiteral("y")] = st.position.y();
        statesArr.append(obj);
    }
    root[QStringLiteral("states")] = statesArr;
    QJsonArray transArr;
    for (const auto &t : m_machine.transitions) {
        QJsonObject obj;
        obj[QStringLiteral("source")] = t.sourceState;
        obj[QStringLiteral("target")] = t.targetState;
        obj[QStringLiteral("event")] = t.event;
        obj[QStringLiteral("guard")] = t.guard;
        obj[QStringLiteral("action")] = t.action;
        transArr.append(obj);
    }
    root[QStringLiteral("transitions")] = transArr;
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

/** @brief 导出Graphviz DOT格式 @return DOT字符串 */
QString StateMachineDesigner::exportToGraphviz() const
{
    QString code;
    QTextStream s(&code);
    s << "digraph " << m_machine.name << " {\n";
    s << "    rankdir=LR;\n";
    s << "    node [shape=record, fontname=\"Arial\"];\n";
    s << "    edge [fontname=\"Arial\"];\n\n";
    /* 初始状态伪节点 */
    QString initName;
    for (const auto &st : m_machine.states) {
        if (st.isInitial) { initName = st.name; break; }
    }
    if (!initName.isEmpty()) {
        s << "    __init__ [shape=point];\n";
        s << "    __init__ -> " << initName << ";\n\n";
    }
    /* 状态节点 */
    for (const auto &st : m_machine.states) {
        QString shape = st.isFinal ? QStringLiteral("doublecircle")
                                   : QStringLiteral("rounded");
        QString label = st.name;
        if (!st.entryAction.isEmpty() || !st.exitAction.isEmpty()) {
            label += QStringLiteral("|");
            if (!st.entryAction.isEmpty()) {
                label += QStringLiteral("entry: ") + st.entryAction;
            }
            if (!st.exitAction.isEmpty()) {
                label += QStringLiteral("\\nexit: ") + st.exitAction;
            }
        }
        s << "    " << st.name << " [shape=" << shape
          << ", label=\"" << label << "\"];\n";
    }
    s << QStringLiteral("\n");
    /* 迁移边 */
    for (const auto &t : m_machine.transitions) {
        s << "    " << t.sourceState << " -> " << t.targetState
          << " [label=\"" << t.event;
        if (!t.guard.isEmpty()) {
            s << " [" << t.guard << "]";
        }
        if (!t.action.isEmpty()) {
            s << " / " << t.action;
        }
        s << "\"];\n";
    }
    s << "}\n";
    return code;
}

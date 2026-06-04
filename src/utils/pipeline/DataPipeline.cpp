/**
 * @file DataPipeline.cpp
 * @brief 数据管道实现 — 多阶段数据处理管线
 *
 * 支持添加/插入/移除/启用/禁用/排序处理阶段，
 * 数据依次通过所有启用的阶段进行变换处理。
 */

#include "utils/pipeline/DataPipeline.h"

/** @brief 构造数据管道 @param parent 父对象 */
DataPipeline::DataPipeline(QObject *parent) : QObject(parent) {}

/** @brief 析构函数 */
DataPipeline::~DataPipeline() = default;

/** @brief 在管道末尾添加一个处理阶段(默认启用) @param name 阶段名称 @param fn 处理函数 */
void DataPipeline::addStage(const QString &name, StageFunc fn) { m_stages.append({name, fn, true}); }

/** @brief 在指定位置插入一个处理阶段(默认启用) @param idx 插入位置索引 @param name 阶段名称 @param fn 处理函数 */
void DataPipeline::insertStage(int idx, const QString &name, StageFunc fn) {
    if (idx >= 0 && idx <= m_stages.size()) m_stages.insert(idx, {name, fn, true});
}

/** @brief 按名称移除第一个匹配的处理阶段 @param name 阶段名称 */
void DataPipeline::removeStage(const QString &name) {
    for (int i = 0; i < m_stages.size(); ++i)
        if (m_stages[i].name == name) { m_stages.removeAt(i); return; }
}

/** @brief 启用或禁用指定名称的处理阶段 @param name 阶段名称 @param enabled true=启用 false=禁用 */
void DataPipeline::enableStage(const QString &name, bool enabled) {
    for (auto &s : m_stages) if (s.name == name) { s.enabled = enabled; return; }
}

/** @brief 移动处理阶段的位置 @param from 原始位置索引 @param to 目标位置索引 */
void DataPipeline::moveStage(int from, int to) {
    if (from >= 0 && from < m_stages.size() && to >= 0 && to < m_stages.size()) {
        m_stages.move(from, to);
    }
}

/** @brief 依次通过所有启用的阶段处理输入数据，每个阶段发射stageProcessed信号 @param input 原始输入数据 @return 处理后的数据 */
QByteArray DataPipeline::process(const QByteArray &input) {
    ++m_totalProcessCalls;
    m_totalBytesProcessed += static_cast<quint64>(input.size());
    QByteArray data = input;
    for (const auto &stage : m_stages) {
        if (!stage.enabled) continue;
        int inSize = data.size();
        try {
            data = stage.fn(data);
            emit stageProcessed(stage.name, inSize, data.size());
        } catch (...) {
            ++m_totalStageErrors;
            emit stageError(stage.name, tr("Processing failed"));
        }
    }
    emit pipelineComplete(data);
    return data;
}

/** @brief 获取所有阶段名称列表(按管道顺序) @return 阶段名称列表 */
QStringList DataPipeline::stageNames() const { QStringList l; for (const auto &s : m_stages) l << s.name; return l; }

/** @brief 获取管道中的阶段总数 @return 阶段数量 */
int DataPipeline::stageCount() const { return m_stages.size(); }

/** @brief 清空所有处理阶段 */
void DataPipeline::clearStages() { m_stages.clear(); }

/**
 * @file DataPipeline.cpp
 * @brief 数据管道实现 — 多阶段数据变换管线，支持动态添加/移除处理阶段
 * @since score-132
 */
#include "core/pipeline/DataPipeline.h"

/** @brief 构造函数 @param parent 父对象指针 */
DataPipeline::DataPipeline(QObject *parent) : QObject(parent) {}

/** @brief 析构函数 */
DataPipeline::~DataPipeline() = default;

/** @brief 获取单例实例 @return DataPipeline全局唯一实例引用 */
DataPipeline &DataPipeline::instance() { static DataPipeline inst; return inst; }

/** @brief 创建命名管线，若已存在则不重复创建 @param name 管线名称 @return 管线名称字符串 */
QString DataPipeline::createPipeline(const QString &name) {
    if (!m_pipelines.contains(name)) {
        m_pipelines[name] = QList<PipelineStage>();
        ++m_totalPipelinesCreated;
        emit pipelineCreated(name);
    }
    return name;
}

/** @brief 移除指定管线及其所有阶段 @param name 管线名称 */
void DataPipeline::removePipeline(const QString &name) {
    if (m_pipelines.contains(name)) {
        ++m_totalPipelineRemovals;
        m_pipelines.remove(name);
    }
    emit pipelineRemoved(name);
}

/** @brief 获取所有已创建管线的名称列表 @return 管线名称字符串列表 */
QStringList DataPipeline::pipelineNames() const { return m_pipelines.keys(); }

/** @brief 向指定管线添加处理阶段，管线不存在时自动创建 @param pipelineName 管线名称 @param stageName 阶段名称 @param transform 数据变换函数对象 */
void DataPipeline::addStage(const QString &pipelineName, const QString &stageName, DataTransform transform) {
    if (!m_pipelines.contains(pipelineName)) createPipeline(pipelineName);
    PipelineStage stage; stage.name = stageName; stage.transform = transform; stage.enabled = true;
    m_pipelines[pipelineName].append(stage);
    ++m_totalStageAdds;
}

/** @brief 从指定管线中移除命名阶段 @param pipelineName 管线名称 @param stageName 阶段名称 */
void DataPipeline::removeStage(const QString &pipelineName, const QString &stageName) {
    if (!m_pipelines.contains(pipelineName)) return;
    auto &stages = m_pipelines[pipelineName];
    int before = stages.size();
    stages.removeIf([&stageName](const PipelineStage &s) { return s.name == stageName; });
    if (stages.size() < before) ++m_totalStageRemoves;
}

/** @brief 设置指定管线中某个阶段的启用状态 @param pipelineName 管线名称 @param stageName 阶段名称 @param enabled true启用，false禁用 */
void DataPipeline::setStageEnabled(const QString &pipelineName, const QString &stageName, bool enabled) {
    if (!m_pipelines.contains(pipelineName)) return;
    for (auto &s : m_pipelines[pipelineName]) { if (s.name == stageName) { s.enabled = enabled; break; } }
}

/** @brief 获取指定管线中所有阶段名称列表 @param pipelineName 管线名称 @return 阶段名称字符串列表 */
QStringList DataPipeline::stageNames(const QString &pipelineName) const {
    QStringList names;
    if (m_pipelines.contains(pipelineName)) { for (const auto &s : m_pipelines[pipelineName]) names.append(s.name); }
    return names;
}

/** @brief 通过指定管线处理数据，依次执行所有启用的阶段 @param pipelineName 管线名称 @param input 输入字节数据 @return 处理后的字节数据 */
QByteArray DataPipeline::process(const QString &pipelineName, const QByteArray &input) {
    ++m_totalProcessed;
    m_totalBytesInput += input.size();
    if (!m_pipelines.contains(pipelineName)) { ++m_totalBypassedProcess; m_totalBytesOutput += input.size(); return input; }
    QByteArray data = input;
    for (const auto &stage : m_pipelines[pipelineName]) {
        if (stage.enabled && stage.transform) { data = stage.transform(data); }
    }
    m_totalBytesOutput += data.size();
    emit dataProcessed(pipelineName, input.size(), data.size());
    return data;
}

/** @brief 使用单次变换函数处理数据(无需管线) @param input 输入字节数据 @param transform 数据变换函数对象 @return 处理后的字节数据 */
QByteArray DataPipeline::processSingle(const QByteArray &input, DataTransform transform) {
    ++m_totalProcessed;
    m_totalBytesInput += input.size();
    QByteArray output = transform ? transform(input) : input;
    m_totalBytesOutput += output.size();
    return output;
}

/** @brief 清空指定管线的所有阶段(保留管线本身) @param name 管线名称 */
void DataPipeline::clearPipeline(const QString &name) { if (m_pipelines.contains(name)) m_pipelines[name].clear(); }

/** @brief 清空所有管线及其阶段 */
void DataPipeline::clearAll() { m_pipelines.clear(); }

/** @brief 获取总处理次数 @return 累计数据通过管线处理的次数 */
quint64 DataPipeline::totalProcessed() const { return m_totalProcessed; }

/** @brief 获取总输入字节数 @return 所有处理请求的输入字节累计 */
quint64 DataPipeline::totalBytesInput() const { return m_totalBytesInput; }

/** @brief 获取总输出字节数 @return 所有处理请求的输出字节累计 */
quint64 DataPipeline::totalBytesOutput() const { return m_totalBytesOutput; }

/** @brief 获取总创建管线数 @return 累计创建管线的次数 */
quint64 DataPipeline::totalPipelinesCreated() const { return m_totalPipelinesCreated; }

/** @brief 重置所有统计计数器 */
void DataPipeline::resetStatistics() {
    m_totalProcessed = 0; m_totalBytesInput = 0; m_totalBytesOutput = 0;
    m_totalPipelinesCreated = 0; m_totalPipelineRemovals = 0;
    m_totalStageAdds = 0; m_totalStageRemoves = 0; m_totalBypassedProcess = 0;
}

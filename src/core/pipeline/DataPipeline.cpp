/**
 * @file DataPipeline.cpp
 * @brief 数据管道实现
 * @since score-132
 */
#include "core/pipeline/DataPipeline.h"

DataPipeline::DataPipeline(QObject *parent) : QObject(parent) {}
DataPipeline::~DataPipeline() = default;
DataPipeline &DataPipeline::instance() { static DataPipeline inst; return inst; }

QString DataPipeline::createPipeline(const QString &name) {
    if (!m_pipelines.contains(name)) {
        m_pipelines[name] = QList<PipelineStage>();
        ++m_totalPipelinesCreated;
        emit pipelineCreated(name);
    }
    return name;
}
void DataPipeline::removePipeline(const QString &name) {
    m_pipelines.remove(name);
    emit pipelineRemoved(name);
}
QStringList DataPipeline::pipelineNames() const { return m_pipelines.keys(); }

void DataPipeline::addStage(const QString &pipelineName, const QString &stageName, DataTransform transform) {
    if (!m_pipelines.contains(pipelineName)) createPipeline(pipelineName);
    PipelineStage stage; stage.name = stageName; stage.transform = transform; stage.enabled = true;
    m_pipelines[pipelineName].append(stage);
}
void DataPipeline::removeStage(const QString &pipelineName, const QString &stageName) {
    if (!m_pipelines.contains(pipelineName)) return;
    auto &stages = m_pipelines[pipelineName];
    stages.removeIf([&stageName](const PipelineStage &s) { return s.name == stageName; });
}
void DataPipeline::setStageEnabled(const QString &pipelineName, const QString &stageName, bool enabled) {
    if (!m_pipelines.contains(pipelineName)) return;
    for (auto &s : m_pipelines[pipelineName]) { if (s.name == stageName) { s.enabled = enabled; break; } }
}
QStringList DataPipeline::stageNames(const QString &pipelineName) const {
    QStringList names;
    if (m_pipelines.contains(pipelineName)) { for (const auto &s : m_pipelines[pipelineName]) names.append(s.name); }
    return names;
}

QByteArray DataPipeline::process(const QString &pipelineName, const QByteArray &input) {
    ++m_totalProcessed;
    m_totalBytesInput += input.size();
    if (!m_pipelines.contains(pipelineName)) { m_totalBytesOutput += input.size(); return input; }
    QByteArray data = input;
    for (const auto &stage : m_pipelines[pipelineName]) {
        if (stage.enabled && stage.transform) { data = stage.transform(data); }
    }
    m_totalBytesOutput += data.size();
    emit dataProcessed(pipelineName, input.size(), data.size());
    return data;
}
QByteArray DataPipeline::processSingle(const QByteArray &input, DataTransform transform) {
    ++m_totalProcessed;
    m_totalBytesInput += input.size();
    QByteArray output = transform ? transform(input) : input;
    m_totalBytesOutput += output.size();
    return output;
}

void DataPipeline::clearPipeline(const QString &name) { if (m_pipelines.contains(name)) m_pipelines[name].clear(); }
void DataPipeline::clearAll() { m_pipelines.clear(); }

quint64 DataPipeline::totalProcessed() const { return m_totalProcessed; }
quint64 DataPipeline::totalBytesInput() const { return m_totalBytesInput; }
quint64 DataPipeline::totalBytesOutput() const { return m_totalBytesOutput; }
quint64 DataPipeline::totalPipelinesCreated() const { return m_totalPipelinesCreated; }
void DataPipeline::resetStatistics() { m_totalProcessed = 0; m_totalBytesInput = 0; m_totalBytesOutput = 0; m_totalPipelinesCreated = 0; }

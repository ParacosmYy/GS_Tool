#include "utils/pipeline/DataPipeline.h"

DataPipeline::DataPipeline(QObject *parent) : QObject(parent) {}
DataPipeline::~DataPipeline() = default;

void DataPipeline::addStage(const QString &name, StageFunc fn) { m_stages.append({name, fn, true}); }
void DataPipeline::insertStage(int idx, const QString &name, StageFunc fn) {
    if (idx >= 0 && idx <= m_stages.size()) m_stages.insert(idx, {name, fn, true});
}
void DataPipeline::removeStage(const QString &name) {
    for (int i = 0; i < m_stages.size(); ++i)
        if (m_stages[i].name == name) { m_stages.removeAt(i); return; }
}
void DataPipeline::enableStage(const QString &name, bool enabled) {
    for (auto &s : m_stages) if (s.name == name) { s.enabled = enabled; return; }
}
void DataPipeline::moveStage(int from, int to) {
    if (from >= 0 && from < m_stages.size() && to >= 0 && to < m_stages.size()) {
        m_stages.move(from, to);
    }
}

QByteArray DataPipeline::process(const QByteArray &input) {
    QByteArray data = input;
    for (const auto &stage : m_stages) {
        if (!stage.enabled) continue;
        int inSize = data.size();
        try {
            data = stage.fn(data);
            emit stageProcessed(stage.name, inSize, data.size());
        } catch (...) {
            emit stageError(stage.name, tr("Processing failed"));
        }
    }
    emit pipelineComplete(data);
    return data;
}

QStringList DataPipeline::stageNames() const { QStringList l; for (const auto &s : m_stages) l << s.name; return l; }
int DataPipeline::stageCount() const { return m_stages.size(); }
void DataPipeline::clearStages() { m_stages.clear(); }

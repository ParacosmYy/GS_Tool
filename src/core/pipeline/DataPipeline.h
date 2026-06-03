/**
 * @file DataPipeline.h
 * @brief 数据管道 - 可组合的数据处理管道框架
 * @since score-132
 */
#ifndef DATAPIPELINE_H
#define DATAPIPELINE_H
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <functional>

using DataTransform = std::function<QByteArray(const QByteArray &)>;

struct PipelineStage {
    QString name;
    DataTransform transform;
    bool enabled = true;
};

class DataPipeline : public QObject {
    Q_OBJECT
public:
    static DataPipeline &instance();

    QString createPipeline(const QString &name);
    void removePipeline(const QString &name);
    QStringList pipelineNames() const;

    void addStage(const QString &pipelineName, const QString &stageName, DataTransform transform);
    void removeStage(const QString &pipelineName, const QString &stageName);
    void setStageEnabled(const QString &pipelineName, const QString &stageName, bool enabled);
    QStringList stageNames(const QString &pipelineName) const;

    QByteArray process(const QString &pipelineName, const QByteArray &input);
    QByteArray processSingle(const QByteArray &input, DataTransform transform);

    void clearPipeline(const QString &pipelineName);
    void clearAll();

    quint64 totalProcessed() const;
    quint64 totalBytesInput() const;
    quint64 totalBytesOutput() const;
    quint64 totalPipelinesCreated() const;
    void resetStatistics();

signals:
    void pipelineCreated(const QString &name);
    void pipelineRemoved(const QString &name);
    void dataProcessed(const QString &pipelineName, int inputSize, int outputSize);

private:
    explicit DataPipeline(QObject *parent = nullptr);
    ~DataPipeline() override;
    DataPipeline(const DataPipeline &) = delete;
    DataPipeline &operator=(const DataPipeline &) = delete;

    QMap<QString, QList<PipelineStage>> m_pipelines;
    mutable quint64 m_totalProcessed = 0;
    mutable quint64 m_totalBytesInput = 0;
    mutable quint64 m_totalBytesOutput = 0;
    quint64 m_totalPipelinesCreated = 0;
};
#endif // DATAPIPELINE_H

#pragma once
#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <functional>

class DataPipeline : public QObject {
    Q_OBJECT
public:
    using StageFunc = std::function<QByteArray(const QByteArray &)>;
    struct Stage { QString name; StageFunc fn; bool enabled = true; };

    explicit DataPipeline(QObject *parent = nullptr);
    ~DataPipeline() override;
    void addStage(const QString &name, StageFunc fn);
    void insertStage(int index, const QString &name, StageFunc fn);
    void removeStage(const QString &name);
    void enableStage(const QString &name, bool enabled);
    void moveStage(int from, int to);
    QByteArray process(const QByteArray &input);
    QStringList stageNames() const;
    int stageCount() const;
    void clearStages();
signals:
    void stageProcessed(const QString &name, int inputSize, int outputSize);
    void pipelineComplete(const QByteArray &finalOutput);
    void stageError(const QString &name, const QString &error);
private:
    QList<Stage> m_stages;
};

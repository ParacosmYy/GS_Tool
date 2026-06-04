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
    /** @brief 获取数据管道单例引用 */
    static DataPipeline &instance();

    /** @brief 创建指定名称的新管道
     *  @param name 管道名称（唯一标识）
     *  @return 管道名称（创建成功）或空串（已存在） */
    QString createPipeline(const QString &name);
    /** @brief 移除指定名称的管道及其所有阶段
     *  @param name 管道名称 */
    void removePipeline(const QString &name);
    /** @brief 获取所有已创建管道的名称列表
     *  @return 管道名称列表 */
    QStringList pipelineNames() const;

    /** @brief 向指定管道添加处理阶段
     *  @param pipelineName 管道名称
     *  @param stageName 阶段名称（管道内唯一）
     *  @param transform 数据转换函数 */
    void addStage(const QString &pipelineName, const QString &stageName, DataTransform transform);
    /** @brief 从指定管道移除处理阶段
     *  @param pipelineName 管道名称
     *  @param stageName 阶段名称 */
    void removeStage(const QString &pipelineName, const QString &stageName);
    /** @brief 设置指定阶段的启用/禁用状态（禁用的阶段跳过执行）
     *  @param pipelineName 管道名称
     *  @param stageName 阶段名称
     *  @param enabled true启用，false禁用 */
    void setStageEnabled(const QString &pipelineName, const QString &stageName, bool enabled);
    /** @brief 获取指定管道的所有阶段名称列表
     *  @param pipelineName 管道名称
     *  @return 阶段名称列表（按添加顺序） */
    QStringList stageNames(const QString &pipelineName) const;

    /** @brief 通过指定管道依次处理输入数据
     *  @param pipelineName 管道名称
     *  @param input 输入字节数据
     *  @return 经过所有启用的阶段处理后的输出数据 */
    QByteArray process(const QString &pipelineName, const QByteArray &input);
    /** @brief 使用单个转换函数处理数据（不创建管道）
     *  @param input 输入字节数据
     *  @param transform 数据转换函数
     *  @return 处理后的输出数据 */
    QByteArray processSingle(const QByteArray &input, DataTransform transform);

    /** @brief 清空指定管道的所有阶段
     *  @param pipelineName 管道名称 */
    void clearPipeline(const QString &pipelineName);
    /** @brief 清空所有管道及其阶段 */
    void clearAll();

    /** @brief 获取累计数据处理总次数 */
    quint64 totalProcessed() const;
    /** @brief 获取累计输入字节总数 */
    quint64 totalBytesInput() const;
    /** @brief 获取累计输出字节总数 */
    quint64 totalBytesOutput() const;
    /** @brief 获取累计创建管道总次数 */
    quint64 totalPipelinesCreated() const;
    /** @brief 获取累计移除管道总次数 */
    quint64 totalPipelineRemovals() const { return m_totalPipelineRemovals; }
    /** @brief 获取累计添加阶段总次数 */
    quint64 totalStageAdds() const { return m_totalStageAdds; }
    /** @brief 获取累计移除阶段总次数 */
    quint64 totalStageRemoves() const { return m_totalStageRemoves; }
    /** @brief 获取累计因阶段禁用而跳过处理的次数 */
    quint64 totalBypassedProcess() const { return m_totalBypassedProcess; }
    /** @brief 重置所有统计计数器 */
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
    quint64 m_totalPipelineRemovals = 0;
    quint64 m_totalStageAdds = 0;
    quint64 m_totalStageRemoves = 0;
    quint64 m_totalBypassedProcess = 0;
};
#endif // DATAPIPELINE_H

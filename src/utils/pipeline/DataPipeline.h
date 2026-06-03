#pragma once
#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <functional>

/**
 * @brief 数据管道 — 多阶段数据处理管线
 *
 * 支持添加/插入/移除/启用/禁用/排序处理阶段，
 * 数据依次通过所有启用的阶段进行变换处理。
 */
class DataPipeline : public QObject {
    Q_OBJECT
public:
    /** @brief 处理阶段函数类型: 输入字节数组，返回处理后的字节数组 */
    using StageFunc = std::function<QByteArray(const QByteArray &)>;
    /** @brief 处理阶段结构: 名称+处理函数+启用标志 */
    struct Stage { QString name; StageFunc fn; bool enabled = true; };

    /** @brief 构造数据管道 @param parent 父对象 */
    explicit DataPipeline(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~DataPipeline() override;
    /** @brief 在管道末尾添加一个处理阶段(默认启用) @param name 阶段名称 @param fn 处理函数 */
    void addStage(const QString &name, StageFunc fn);
    /** @brief 在指定位置插入一个处理阶段(默认启用) @param index 插入位置索引 @param name 阶段名称 @param fn 处理函数 */
    void insertStage(int index, const QString &name, StageFunc fn);
    /** @brief 按名称移除第一个匹配的处理阶段 @param name 阶段名称 */
    void removeStage(const QString &name);
    /** @brief 启用或禁用指定名称的处理阶段 @param name 阶段名称 @param enabled true=启用 false=禁用 */
    void enableStage(const QString &name, bool enabled);
    /** @brief 移动处理阶段的位置 @param from 原始位置索引 @param to 目标位置索引 */
    void moveStage(int from, int to);
    /** @brief 依次通过所有启用的阶段处理输入数据 @param input 原始输入数据 @return 处理后的数据 */
    QByteArray process(const QByteArray &input);
    /** @brief 获取所有阶段名称列表(按管道顺序) @return 阶段名称列表 */
    QStringList stageNames() const;
    /** @brief 获取管道中的阶段总数 @return 阶段数量 */
    int stageCount() const;
    /** @brief 清空所有处理阶段 */
    void clearStages();
signals:
    /** @brief 单个阶段处理完成信号 @param name 阶段名称 @param inputSize 输入数据大小 @param outputSize 输出数据大小 */
    void stageProcessed(const QString &name, int inputSize, int outputSize);
    /** @brief 管线全部处理完成信号 @param finalOutput 最终输出数据 */
    void pipelineComplete(const QByteArray &finalOutput);
    /** @brief 阶段处理错误信号 @param name 阶段名称 @param error 错误描述 */
    void stageError(const QString &name, const QString &error);
private:
    QList<Stage> m_stages; ///< 处理阶段列表
};

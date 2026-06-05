/**
 * @file ProtocolSequencer.h
 * @brief 协议序列器引擎 -- 多步骤协议序列的加载、执行、暂停/恢复
 *
 * 核心执行引擎，负责:
 *   1. loadSequence() / saveSequence() -- JSON 格式序列的读写
 *   2. execute()  -- 逐步执行序列(Send/Receive/Delay/Loop 等)
 *   3. stop() / pause() / resume() -- 执行生命周期控制
 *   4. 统计累计(sequences executed, steps completed, errors, duration)
 *
 * 协作: SequenceEditorWidget(UI)/ConnectionController(数据通道)
 */

#ifndef PROTOCOLSEQUENCER_H
#define PROTOCOLSEQUENCER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>

#include "protocol/sequencer/SequencerTypes.h"

/**
 * @brief 协议序列器引擎
 *
 * 逐步执行 SequenceStep 列表，通过 sendData 信号对外输出数据，
 * 通过 feedReceivedData() 槽接收响应数据。
 * 支持暂停/恢复/停止，支持循环子序列。
 */
class ProtocolSequencer : public QObject {
    Q_OBJECT

public:
    //-- 构造/析构 --//
    explicit ProtocolSequencer(QObject* parent = nullptr);
    ~ProtocolSequencer() override;

    //-- 序列管理 --//
    /**
     * @brief 加载序列步骤列表
     * @param steps 步骤列表(替换当前序列)
     */
    void setSequence(const QList<SequenceStep>& steps);

    /** @brief 获取当前序列步骤列表 */
    QList<SequenceStep> sequence() const;

    /**
     * @brief 从 JSON 对象加载序列
     * @param json 包含 "steps" 数组的 JSON 对象
     * @return true=加载成功
     */
    bool loadSequence(const QJsonObject& json);

    /**
     * @brief 保存序列到 JSON 对象
     * @return 包含 "steps" 数组的 JSON 对象
     */
    QJsonObject saveSequence() const;

    /**
     * @brief 从文件加载序列
     * @param filePath JSON 文件路径
     * @return true=加载成功
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief 保存序列到文件
     * @param filePath JSON 文件路径
     * @return true=保存成功
     */
    bool saveToFile(const QString& filePath) const;

    //-- 执行控制 --//
    /** @brief 开始执行当前序列 */
    void execute();
    /** @brief 停止执行(不可恢复) */
    void stop();
    /** @brief 暂停执行 */
    void pause();
    /** @brief 恢复执行 */
    void resume();

    /** @brief 是否正在执行 */
    bool isRunning() const;
    /** @brief 是否已暂停 */
    bool isPaused() const;

    //-- 数据输入 --//
    /**
     * @brief 喂入接收到的数据(用于 Receive/WaitFor/Check 步骤)
     * @param data 接收到的原始字节
     */
    void feedReceivedData(const QByteArray& data);

    //-- 统计 --//
    const SequencerStats& stats() const;
    void resetStatistics();

signals:
    /** @brief 步骤开始执行 @param index 步骤索引 */
    void stepStarted(int index);
    /** @brief 步骤执行完成 @param index 步骤索引 @param success 是否成功 */
    void stepCompleted(int index, bool success);
    /** @brief 序列执行完成 @param result 汇总结果 */
    void sequenceComplete(const SequenceResult& result);
    /** @brief 需要发送数据 @param data 待发送字节 */
    void sendData(const QByteArray& data);
    /** @brief 执行日志输出 @param message 日志文本 */
    void logMessage(const QString& message);

private:
    //-- 内部执行引擎 --//
    void executeNextStep();
    void executeSendStep(const SequenceStep& step);
    void executeReceiveStep(const SequenceStep& step);
    void executeDelayStep(const SequenceStep& step);
    void executeLoopStep(const SequenceStep& step);
    void executeCheckStep(const SequenceStep& step);
    void finishSequence(bool success, const QString& error = QString());

    //-- JSON 序列化辅助 --//
    QJsonObject stepToJson(const SequenceStep& step) const;
    SequenceStep jsonToStep(const QJsonObject& obj) const;
    static QString stepTypeToString(StepType type);
    static StepType stringToStepType(const QString& str);

    //-- 成员变量 --//
    QList<SequenceStep> m_steps;            ///< 当前序列步骤列表
    int m_currentIndex = -1;                ///< 当前执行到的步骤索引
    bool m_running = false;                 ///< 是否正在执行
    bool m_paused = false;                  ///< 是否已暂停
    QByteArray m_receivedBuffer;            ///< 接收数据缓冲区
    QTimer* m_delayTimer = nullptr;         ///< 延时定时器
    QTimer* m_receiveTimeoutTimer = nullptr;///< 接收超时定时器
    QElapsedTimer m_sequenceTimer;          ///< 序列执行计时器
    SequenceResult m_currentResult;         ///< 当前执行的中间结果
    SequencerStats m_stats;                 ///< 累计统计

    //-- 循环状态栈 --//
    struct LoopState {
        int startIndex;         ///< 循环体起始步骤索引
        int endIndex;           ///< 循环体结束步骤索引
        int remaining;          ///< 剩余迭代次数
    };
    QList<LoopState> m_loopStack;           ///< 循环嵌套栈
};

#endif // PROTOCOLSEQUENCER_H

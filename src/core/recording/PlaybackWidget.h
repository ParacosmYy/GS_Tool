/**
 * @file PlaybackWidget.h
 * @brief 回放控制面板控件，提供播放/暂停/进度条/倍速选择等 UI
 *
 * 作为 F1 回放子系统的可视化控制面板，用户通过此控件
 * 触发播放、暂停、定位和变速操作。
 */

#ifndef PLAYBACK_WIDGET_H
#define PLAYBACK_WIDGET_H

#include <QWidget>

class QPushButton;
class QSlider;
class QComboBox;
class QLabel;

/**
 * @class PlaybackWidget
 * @brief 回放控制面板
 *
 * 包含播放/暂停按钮、进度滑块、倍速下拉框和时间显示标签，
 * 通过信号将用户操作转发给 PlaybackController。
 */
class PlaybackWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父控件指针
     */
    explicit PlaybackWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置回放总时长
     * @param durationMs 总时长（毫秒）
     */
    void setDuration(qint64 durationMs);

    /**
     * @brief 设置当前回放时间
     * @param timeMs 当前时间（毫秒）
     */
    void setCurrentTime(qint64 timeMs);

signals:
    /**
     * @brief 用户请求播放信号
     */
    void playRequested();

    /**
     * @brief 用户请求暂停信号
     */
    void pauseRequested();

    /**
     * @brief 用户请求停止信号
     */
    void stopRequested();

    /**
     * @brief 用户请求定位信号
     * @param timeMs 目标时间（毫秒）
     */
    void seekRequested(qint64 timeMs);

    /**
     * @brief 用户请求变更倍速信号
     * @param speed 新的倍速值
     */
    void speedChangeRequested(qreal speed);

private:
    /**
     * @brief 初始化界面布局和控件
     */
    void setupUI();

    QPushButton* m_playBtn;     ///< 播放/暂停按钮
    QSlider*     m_slider;      ///< 进度滑块
    QComboBox*   m_speedCombo;  ///< 倍速选择下拉框
    QLabel*      m_timeLabel;   ///< 时间显示标签
};

#endif // PLAYBACK_WIDGET_H

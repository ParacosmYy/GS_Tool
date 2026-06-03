#pragma once
#include <QWidget>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QTimer>

class ScriptRecorder : public QWidget {
    Q_OBJECT
public:
    struct RecordEntry { qint64 timestamp; QString action; QByteArray data; };
    explicit ScriptRecorder(QWidget *parent = nullptr);
    ~ScriptRecorder() override;
    void startRecording();
    void stopRecording();
    bool isRecording() const;
    void recordAction(const QString &action, const QByteArray &data);
    void playback();
    void clear();
    QList<RecordEntry> entries() const;
    int entryCount() const;
    void setPlaybackSpeed(double speed);
signals:
    void recordingStarted();
    void recordingStopped(int entryCount);
    void actionRecorded(const QString &action);
    void playbackStarted();
    void playbackFinished();
    void playbackAction(const QString &action, const QByteArray &data);
private:
    void onPlaybackTick();
    QList<RecordEntry> m_entries;
    bool m_recording = false;
    bool m_playing = false;
    double m_speed = 1.0;
    QTimer *m_timer = nullptr;
    int m_playIndex = 0;
};

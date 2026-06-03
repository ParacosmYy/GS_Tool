#pragma once
#include <QWidget>
#include <QByteArray>
#include <QString>

class QLabel;
class MidiWidget : public QWidget {
    Q_OBJECT
public:
    explicit MidiWidget(QWidget *parent = nullptr);
    ~MidiWidget() override;
    void appendMessage(const QByteArray &midiData);
    void setChannelFilter(int channel);
    void clearDisplay();
    int messageCount() const;
signals:
    void messageReceived(const QString &formatted);
    void noteOn(int channel, int note, int velocity);
    void noteOff(int channel, int note);
    void controlChange(int channel, int controller, int value);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void setupUi();
    QString formatMidi(const QByteArray &data) const;
    void parseMidi(const QByteArray &data);
    QLabel *m_statusLabel = nullptr;
    QLabel *m_displayLabel = nullptr;
    int m_channelFilter = -1;
    int m_msgCount = 0;
};

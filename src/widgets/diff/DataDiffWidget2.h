#pragma once
#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QList>

class DataDiffWidget : public QWidget {
    Q_OBJECT
public:
    struct DiffLine { int leftLine; int rightLine; QByteArray leftData; QByteArray rightData; bool different; };
    explicit DataDiffWidget(QWidget *parent = nullptr);
    ~DataDiffWidget() override;
    void setLeftData(const QByteArray &data);
    void setRightData(const QByteArray &data);
    void setBytesPerLine(int bytes);
    QList<DiffLine> computeDiff() const;
    void clear();
    int diffCount() const;
signals:
    void diffComputed(int diffLines, int totalLines);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QByteArray m_left;
    QByteArray m_right;
    int m_bytesPerLine = 16;
    int m_diffCount = 0;
};

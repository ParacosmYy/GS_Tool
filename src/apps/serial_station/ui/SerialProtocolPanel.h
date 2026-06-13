#ifndef SERIAL_PROTOCOL_PANEL_H
#define SERIAL_PROTOCOL_PANEL_H

#include <QtCore/QStringList>
#include <QtWidgets/QWidget>

class QComboBox;
class QLabel;

namespace serial_station {

/**
 * @brief Serial Station 协议选择面板。
 *
 * 只展示协议列表并发出选择意图，不直接访问协议 registry 或串口 core。
 */
class SerialProtocolPanel : public QWidget {
    Q_OBJECT

public:
    explicit SerialProtocolPanel(QWidget* parent = nullptr);

    QString activeProtocol() const;

public slots:
    void setProtocols(const QStringList& protocolNames, const QString& activeProtocol);
    void setActiveProtocol(const QString& protocolName);

signals:
    void protocolSelected(const QString& protocolName);

private slots:
    void emitProtocolSelected(int index);

private:
    void setupUi();
    void connectSignals();
    void refreshStatus();

    QComboBox* m_protocolCombo = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_hintLabel = nullptr;
};

} // namespace serial_station

#endif // SERIAL_PROTOCOL_PANEL_H

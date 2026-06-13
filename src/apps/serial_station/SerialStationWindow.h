#ifndef SERIAL_STATION_WINDOW_H
#define SERIAL_STATION_WINDOW_H

#include <QtWidgets/QWidget>

#include <QtCore/QStringList>
#include <memory>

namespace serial_station {

class SerialCommandPanel;
class SerialLogPanel;
class SerialProtocolPanel;
struct SerialProfileResult;
struct SerialProfileWriteResult;
class SerialStationController;
struct SerialStationProfile;
class SerialStatusBar;
class SerialPortPanel;

/**
 * @brief Serial Station 顶层窗口骨架。
 *
 * 当前只负责持有 controller 和窗口对象名，不承载业务逻辑。
 */
class SerialStationWindow : public QWidget {
    Q_OBJECT

public:
    explicit SerialStationWindow(QWidget* parent = nullptr);
    ~SerialStationWindow() override;

    /**
     * @brief 保存当前工作台配置档案，供自动化测试和 UI 入口复用。
     */
    SerialProfileWriteResult saveCurrentProfileToFile(const QString& filePath,
                                                      const QString& name,
                                                      const QString& description = QString(),
                                                      const QStringList& tags = QStringList());

    /**
     * @brief 从文件加载配置档案并应用到工作台 UI。
     */
    SerialProfileResult loadProfileFromFile(const QString& filePath);

private:
    SerialStationProfile collectCurrentProfile(const QString& name,
                                               const QString& description,
                                               const QStringList& tags) const;
    void applyProfileToUi(const SerialStationProfile& profile);
    void saveProfileWithDialog();
    void loadProfileWithDialog();

    std::unique_ptr<SerialStationController> m_controller;
    SerialPortPanel* m_portPanel = nullptr;
    SerialProtocolPanel* m_protocolPanel = nullptr;
    SerialCommandPanel* m_commandPanel = nullptr;
    SerialLogPanel* m_logPanel = nullptr;
    SerialStatusBar* m_statusBar = nullptr;
};

} // namespace serial_station

#endif // SERIAL_STATION_WINDOW_H

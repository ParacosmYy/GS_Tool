#ifndef SERIAL_STATION_WINDOW_H
#define SERIAL_STATION_WINDOW_H

#include <QtWidgets/QWidget>

#include <QtCore/QStringList>
#include <memory>

class QComboBox;
class QPushButton;

namespace serial_station {

class SerialCommandPanel;
class SerialLogPanel;
class SerialMeasurementPanel;
class SerialProtocolPanel;
class SerialProfileCatalogService;
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

    /**
     * @brief 设置保存/加载配置档案对话框的默认目录。
     * @param directoryPath 目录路径；为空时恢复系统默认目录
     */
    void setDefaultProfileDirectory(const QString& directoryPath);

    /**
     * @brief 当前保存/加载配置档案对话框的默认目录。
     */
    QString defaultProfileDirectory() const;

    /**
     * @brief 启动阶段加载配置档案。
     * @return true 表示档案已成功应用；失败时保持工站打开供用户修正
     */
    bool loadStartupProfile(const QString& filePath);

    /**
     * @brief 启动阶段加载最近一次成功使用的配置档案。
     * @return true 表示上次档案已成功应用
     */
    bool loadStartupLastProfile();

    /**
     * @brief 最近成功使用过的配置档案路径。
     */
    QStringList recentProfilePaths() const;

    /**
     * @brief 最近一次成功使用的配置档案路径。
     */
    QString lastProfilePath() const;

    /**
     * @brief 重新加载最近一次成功使用的配置档案。
     * @return true 表示上次档案已成功重新应用
     */
    bool reloadLastProfile();

    /**
     * @brief 清理最近档案中已不存在的路径。
     * @return 被清理的失效档案数量
     */
    int pruneMissingProfiles();

    /**
     * @brief 清空最近配置档案索引。
     * @return true 表示清空前存在最近档案
     */
    bool clearRecentProfiles();

    /**
     * @brief 从当前默认档案目录导入配置档案路径。
     * @return 本次新纳入最近列表的档案数量
     */
    int importProfilesFromDefaultDirectory();

private:
    SerialStationProfile collectCurrentProfile(const QString& name,
                                               const QString& description,
                                               const QStringList& tags) const;
    void applyProfileToUi(const SerialStationProfile& profile);
    void recordSuccessfulProfilePath(const QString& filePath);
    void refreshProfileCatalogUi();
    int pruneMissingProfiles(bool logWhenEmpty);
    void saveProfileWithDialog();
    void loadProfileWithDialog();
    void loadSelectedRecentProfile(int index);

    std::unique_ptr<SerialStationController> m_controller;
    std::unique_ptr<SerialProfileCatalogService> m_profileCatalog;
    SerialPortPanel* m_portPanel = nullptr;
    SerialProtocolPanel* m_protocolPanel = nullptr;
    SerialMeasurementPanel* m_measurementPanel = nullptr;
    SerialCommandPanel* m_commandPanel = nullptr;
    SerialLogPanel* m_logPanel = nullptr;
    SerialStatusBar* m_statusBar = nullptr;
    QString m_defaultProfileDirectory; ///< 档案保存/加载对话框默认目录
    QComboBox* m_recentProfileCombo = nullptr;
    QPushButton* m_reloadLastProfileButton = nullptr;
    QPushButton* m_pruneMissingProfilesButton = nullptr;
    QPushButton* m_clearRecentProfilesButton = nullptr;
};

} // namespace serial_station

#endif // SERIAL_STATION_WINDOW_H

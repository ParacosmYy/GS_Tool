#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariantMap>

class ProfileManager : public QObject {
    Q_OBJECT
public:
    struct DeviceProfile { QString id; QString name; QString type; QVariantMap settings; bool isDefault = false; };
    explicit ProfileManager(QObject *parent = nullptr);
    ~ProfileManager() override;
    QString createProfile(const QString &name, const QString &type);
    void deleteProfile(const QString &id);
    void updateProfile(const QString &id, const QVariantMap &settings);
    void renameProfile(const QString &id, const QString &newName);
    DeviceProfile profile(const QString &id) const;
    QList<DeviceProfile> profiles() const;
    QList<DeviceProfile> profilesByType(const QString &type) const;
    void setDefault(const QString &id);
    DeviceProfile defaultProfile() const;
    int count() const;
    void importProfile(const QVariantMap &data);
    QVariantMap exportProfile(const QString &id) const;
signals:
    void profileCreated(const QString &id);
    void profileDeleted(const QString &id);
    void profileUpdated(const QString &id);
    void defaultChanged(const QString &id);
private:
    QMap<QString, DeviceProfile> m_profiles;
    QString m_defaultId;
    int m_counter = 0;
};

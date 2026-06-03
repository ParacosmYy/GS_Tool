#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QFont>
#include <QFontDatabase>

class FontManager : public QObject {
    Q_OBJECT
public:
    enum PresetFont { Title, Heading, Body, Caption, Code, Monospace };
    Q_ENUM(PresetFont)

    explicit FontManager(QObject *parent = nullptr);
    ~FontManager() override;
    bool loadFont(const QString &name, const QString &filePath);
    QFont font(PresetFont preset) const;
    QFont font(const QString &name, int pointSize = -1) const;
    void setFont(PresetFont preset, const QFont &font);
    void setFontSize(PresetFont preset, int pointSize);
    void setDefaultFamily(const QString &family);
    QString defaultFamily() const;
    QStringList loadedFonts() const;
    QStringList availableFamilies() const;
    void resetToDefaults();
signals:
    void fontLoaded(const QString &name);
    void fontChanged(PresetFont preset);
private:
    void initDefaults();
    QMap<PresetFont, QFont> m_presets;
    QMap<QString, int> m_loadedIds;
    QString m_defaultFamily;
};

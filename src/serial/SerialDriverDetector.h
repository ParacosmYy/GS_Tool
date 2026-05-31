#ifndef SERIALDRIVERDETECTOR_H
#define SERIALDRIVERDETECTOR_H

#include <QStringList>
#include <QVector>

struct DriverInfo {
    QString driverName;    // e.g. "CH340", "CP2102"
    QString description;   // e.g. "WCH CH340 Serial Adapter"
    bool installed;
};

class SerialDriverDetector {
public:
    // Detect all known serial adapter drivers on the system
    static QVector<DriverInfo> detectDrivers();

    // Check if any serial port driver is installed
    static bool hasAnyDriverInstalled();

    // Get human-readable driver status summary
    static QString driverStatusSummary();

private:
    static const QStringList kKnownDrivers;
};

#endif

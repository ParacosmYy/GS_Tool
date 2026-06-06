#include "e9424/m9424.h"
QVector<double> m9424::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

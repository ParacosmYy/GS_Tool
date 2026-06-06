#include "m13212/m13212.h"
QVector<double> m13212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

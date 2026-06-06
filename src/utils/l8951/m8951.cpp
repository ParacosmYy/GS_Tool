#include "l8951/m8951.h"
QVector<double> m8951::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

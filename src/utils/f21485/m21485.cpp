#include "f21485/m21485.h"
QVector<double> m21485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f29485/m29485.h"
QVector<double> m29485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

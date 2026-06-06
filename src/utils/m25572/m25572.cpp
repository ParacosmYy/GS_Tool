#include "m25572/m25572.h"
QVector<double> m25572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

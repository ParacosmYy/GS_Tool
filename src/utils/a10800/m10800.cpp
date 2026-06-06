#include "a10800/m10800.h"
QVector<double> m10800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

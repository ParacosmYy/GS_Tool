#include "a10200/m10200.h"
QVector<double> m10200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "h15127/m15127.h"
QVector<double> m15127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

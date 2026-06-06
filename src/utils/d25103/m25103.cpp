#include "d25103/m25103.h"
QVector<double> m25103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

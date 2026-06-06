#include "m16872/m16872.h"
QVector<double> m16872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

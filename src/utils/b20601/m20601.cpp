#include "b20601/m20601.h"
QVector<double> m20601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

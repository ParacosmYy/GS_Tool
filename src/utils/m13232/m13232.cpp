#include "m13232/m13232.h"
QVector<double> m13232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

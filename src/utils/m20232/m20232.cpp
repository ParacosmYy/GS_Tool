#include "m20232/m20232.h"
QVector<double> m20232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

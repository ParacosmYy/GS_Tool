#include "m13832/m13832.h"
QVector<double> m13832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

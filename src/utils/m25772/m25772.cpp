#include "m25772/m25772.h"
QVector<double> m25772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

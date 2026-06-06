#include "m18432/m18432.h"
QVector<double> m18432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

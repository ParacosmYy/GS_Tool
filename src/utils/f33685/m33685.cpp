#include "f33685/m33685.h"
QVector<double> m33685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "d25223/m25223.h"
QVector<double> m25223::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "r25057/m25057.h"
QVector<double> m25057::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

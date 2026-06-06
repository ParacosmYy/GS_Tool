#include "b25381/m25381.h"
QVector<double> m25381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "d18643/m18643.h"
QVector<double> m18643::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "b26061/m26061.h"
QVector<double> m26061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

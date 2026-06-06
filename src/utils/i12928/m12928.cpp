#include "i12928/m12928.h"
QVector<double> m12928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f16125/m16125.h"
QVector<double> m16125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

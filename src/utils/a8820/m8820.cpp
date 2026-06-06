#include "a8820/m8820.h"
QVector<double> m8820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

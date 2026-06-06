#include "c8602/m8602.h"
QVector<double> m8602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

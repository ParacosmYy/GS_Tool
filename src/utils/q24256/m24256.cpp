#include "q24256/m24256.h"
QVector<double> m24256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

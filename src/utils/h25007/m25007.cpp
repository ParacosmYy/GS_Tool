#include "h25007/m25007.h"
QVector<double> m25007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

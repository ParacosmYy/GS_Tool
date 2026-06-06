#include "h15007/m15007.h"
QVector<double> m15007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

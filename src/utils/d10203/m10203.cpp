#include "d10203/m10203.h"
QVector<double> m10203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

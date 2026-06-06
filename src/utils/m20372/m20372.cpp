#include "m20372/m20372.h"
QVector<double> m20372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

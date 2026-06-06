#include "h25887/m25887.h"
QVector<double> m25887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

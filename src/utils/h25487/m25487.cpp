#include "h25487/m25487.h"
QVector<double> m25487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

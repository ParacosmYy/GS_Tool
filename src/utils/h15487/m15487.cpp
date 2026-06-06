#include "h15487/m15487.h"
QVector<double> m15487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

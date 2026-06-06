#include "l7871/m7871.h"
QVector<double> m7871::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

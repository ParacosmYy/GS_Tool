#include "d25203/m25203.h"
QVector<double> m25203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

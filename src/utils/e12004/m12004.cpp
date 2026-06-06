#include "e12004/m12004.h"
QVector<double> m12004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

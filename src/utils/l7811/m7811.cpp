#include "l7811/m7811.h"
QVector<double> m7811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "l20811/m20811.h"
QVector<double> m20811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "l18811/m18811.h"
QVector<double> m18811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

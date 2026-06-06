#include "l36811/m36811.h"
QVector<double> m36811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

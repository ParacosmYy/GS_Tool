#include "k15670/m15670.h"
QVector<double> m15670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

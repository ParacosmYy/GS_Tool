#include "l15351/m15351.h"
QVector<double> m15351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

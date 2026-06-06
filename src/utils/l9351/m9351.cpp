#include "l9351/m9351.h"
QVector<double> m9351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

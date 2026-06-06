#include "h15387/m15387.h"
QVector<double> m15387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

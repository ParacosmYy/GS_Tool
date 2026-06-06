#include "m10712/m10712.h"
QVector<double> m10712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

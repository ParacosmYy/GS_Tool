#include "h25387/m25387.h"
QVector<double> m25387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

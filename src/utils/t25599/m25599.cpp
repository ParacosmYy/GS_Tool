#include "t25599/m25599.h"
QVector<double> m25599::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

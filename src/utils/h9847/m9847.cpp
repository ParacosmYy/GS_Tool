#include "h9847/m9847.h"
QVector<double> m9847::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

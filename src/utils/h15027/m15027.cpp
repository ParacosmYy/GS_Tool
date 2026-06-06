#include "h15027/m15027.h"
QVector<double> m15027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

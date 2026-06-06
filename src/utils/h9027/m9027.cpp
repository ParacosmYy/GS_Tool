#include "h9027/m9027.h"
QVector<double> m9027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

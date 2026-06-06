#include "h35027/m35027.h"
QVector<double> m35027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "h25027/m25027.h"
QVector<double> m25027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

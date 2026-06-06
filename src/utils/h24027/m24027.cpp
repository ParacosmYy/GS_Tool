#include "h24027/m24027.h"
QVector<double> m24027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

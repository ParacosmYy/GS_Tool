#include "h18027/m18027.h"
QVector<double> m18027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

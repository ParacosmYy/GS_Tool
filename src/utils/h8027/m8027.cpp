#include "h8027/m8027.h"
QVector<double> m8027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

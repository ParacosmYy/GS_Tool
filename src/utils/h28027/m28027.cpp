#include "h28027/m28027.h"
QVector<double> m28027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

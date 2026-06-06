#include "h32027/m32027.h"
QVector<double> m32027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

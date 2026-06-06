#include "k20390/m20390.h"
QVector<double> m20390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

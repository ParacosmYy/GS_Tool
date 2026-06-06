#include "a18500/m18500.h"
QVector<double> m18500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

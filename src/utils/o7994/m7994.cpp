#include "o7994/m7994.h"
QVector<double> m7994::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

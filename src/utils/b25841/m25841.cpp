#include "b25841/m25841.h"
QVector<double> m25841::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

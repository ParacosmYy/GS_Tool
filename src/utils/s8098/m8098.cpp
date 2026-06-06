#include "s8098/m8098.h"
QVector<double> m8098::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "s16098/m16098.h"
QVector<double> m16098::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

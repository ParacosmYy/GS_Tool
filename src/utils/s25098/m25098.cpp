#include "s25098/m25098.h"
QVector<double> m25098::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

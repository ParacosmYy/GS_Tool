#include "m33532/m33532.h"
QVector<double> m33532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

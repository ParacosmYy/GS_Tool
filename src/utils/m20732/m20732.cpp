#include "m20732/m20732.h"
QVector<double> m20732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

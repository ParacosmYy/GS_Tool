#include "e7964/m7964.h"
QVector<double> m7964::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

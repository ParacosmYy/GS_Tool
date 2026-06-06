#include "a7960/m7960.h"
QVector<double> m7960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

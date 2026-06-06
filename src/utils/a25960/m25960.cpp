#include "a25960/m25960.h"
QVector<double> m25960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

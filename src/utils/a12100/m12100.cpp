#include "a12100/m12100.h"
QVector<double> m12100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

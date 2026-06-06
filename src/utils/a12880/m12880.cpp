#include "a12880/m12880.h"
QVector<double> m12880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

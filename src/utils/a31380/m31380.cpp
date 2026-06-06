#include "a31380/m31380.h"
QVector<double> m31380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

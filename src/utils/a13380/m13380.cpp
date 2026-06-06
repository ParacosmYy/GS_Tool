#include "a13380/m13380.h"
QVector<double> m13380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

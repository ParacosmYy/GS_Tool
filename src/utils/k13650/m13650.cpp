#include "k13650/m13650.h"
QVector<double> m13650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

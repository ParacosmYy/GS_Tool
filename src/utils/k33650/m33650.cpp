#include "k33650/m33650.h"
QVector<double> m33650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

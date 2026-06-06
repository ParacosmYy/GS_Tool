#include "b25281/m25281.h"
QVector<double> m25281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

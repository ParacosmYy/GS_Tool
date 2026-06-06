#include "a18080/m18080.h"
QVector<double> m18080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

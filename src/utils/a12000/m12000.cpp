#include "a12000/m12000.h"
QVector<double> m12000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

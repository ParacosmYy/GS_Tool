#include "m7952/m7952.h"
QVector<double> m7952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "b12741/m12741.h"
QVector<double> m12741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "i25208/m25208.h"
QVector<double> m25208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

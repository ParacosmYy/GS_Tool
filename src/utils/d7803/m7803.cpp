#include "d7803/m7803.h"
QVector<double> m7803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "d17803/m17803.h"
QVector<double> m17803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "s16878/m16878.h"
QVector<double> m16878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

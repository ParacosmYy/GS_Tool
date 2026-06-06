#include "b7881/m7881.h"
QVector<double> m7881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m26052/m26052.h"
QVector<double> m26052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

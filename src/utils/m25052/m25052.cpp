#include "m25052/m25052.h"
QVector<double> m25052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

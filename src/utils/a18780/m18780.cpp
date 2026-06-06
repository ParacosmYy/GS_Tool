#include "a18780/m18780.h"
QVector<double> m18780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

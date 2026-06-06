#include "p18555/m18555.h"
QVector<double> m18555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

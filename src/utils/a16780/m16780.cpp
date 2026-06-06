#include "a16780/m16780.h"
QVector<double> m16780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

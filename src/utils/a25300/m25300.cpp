#include "a25300/m25300.h"
QVector<double> m25300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

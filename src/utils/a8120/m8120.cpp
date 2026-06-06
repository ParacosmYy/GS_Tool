#include "a8120/m8120.h"
QVector<double> m8120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

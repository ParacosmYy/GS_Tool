#include "e15804/m15804.h"
QVector<double> m15804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

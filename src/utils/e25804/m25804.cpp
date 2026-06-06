#include "e25804/m25804.h"
QVector<double> m25804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

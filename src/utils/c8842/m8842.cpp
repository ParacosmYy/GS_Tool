#include "c8842/m8842.h"
QVector<double> m8842::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

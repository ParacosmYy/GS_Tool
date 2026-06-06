#include "c8822/m8822.h"
QVector<double> m8822::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

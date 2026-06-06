#include "r9057/m9057.h"
QVector<double> m9057::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

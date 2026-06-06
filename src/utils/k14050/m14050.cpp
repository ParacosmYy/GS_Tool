#include "k14050/m14050.h"
QVector<double> m14050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

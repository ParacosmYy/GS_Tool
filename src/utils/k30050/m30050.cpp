#include "k30050/m30050.h"
QVector<double> m30050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

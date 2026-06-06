#include "k11050/m11050.h"
QVector<double> m11050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

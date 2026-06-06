#include "k20050/m20050.h"
QVector<double> m20050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

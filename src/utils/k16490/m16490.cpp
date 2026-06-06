#include "k16490/m16490.h"
QVector<double> m16490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

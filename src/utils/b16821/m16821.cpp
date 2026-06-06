#include "b16821/m16821.h"
QVector<double> m16821::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

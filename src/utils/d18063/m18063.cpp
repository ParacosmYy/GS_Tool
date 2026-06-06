#include "d18063/m18063.h"
QVector<double> m18063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

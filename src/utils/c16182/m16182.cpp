#include "c16182/m16182.h"
QVector<double> m16182::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

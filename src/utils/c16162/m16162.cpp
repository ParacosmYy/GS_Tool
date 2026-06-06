#include "c16162/m16162.h"
QVector<double> m16162::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

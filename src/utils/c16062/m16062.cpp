#include "c16062/m16062.h"
QVector<double> m16062::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "l37211/m37211.h"
QVector<double> m37211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "l17511/m17511.h"
QVector<double> m17511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

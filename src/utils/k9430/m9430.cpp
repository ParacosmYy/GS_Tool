#include "k9430/m9430.h"
QVector<double> m9430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

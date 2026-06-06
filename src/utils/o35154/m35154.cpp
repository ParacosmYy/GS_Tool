#include "o35154/m35154.h"
QVector<double> m35154::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "g9606/m9606.h"
QVector<double> m9606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

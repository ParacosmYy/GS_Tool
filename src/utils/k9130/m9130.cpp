#include "k9130/m9130.h"
QVector<double> m9130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

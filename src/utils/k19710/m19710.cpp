#include "k19710/m19710.h"
QVector<double> m19710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

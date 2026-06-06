#include "g28386/m28386.h"
QVector<double> m28386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

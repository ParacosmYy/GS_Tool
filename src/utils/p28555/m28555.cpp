#include "p28555/m28555.h"
QVector<double> m28555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

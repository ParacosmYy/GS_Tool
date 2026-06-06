#include "g28566/m28566.h"
QVector<double> m28566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

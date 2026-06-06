#include "g28966/m28966.h"
QVector<double> m28966::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "g28606/m28606.h"
QVector<double> m28606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

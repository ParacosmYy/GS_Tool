#include "h28007/m28007.h"
QVector<double> m28007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

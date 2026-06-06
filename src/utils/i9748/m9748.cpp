#include "i9748/m9748.h"
QVector<double> m9748::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

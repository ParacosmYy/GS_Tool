#include "o9854/m9854.h"
QVector<double> m9854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

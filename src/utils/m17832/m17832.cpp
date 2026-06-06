#include "m17832/m17832.h"
QVector<double> m17832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

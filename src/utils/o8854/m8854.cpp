#include "o8854/m8854.h"
QVector<double> m8854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

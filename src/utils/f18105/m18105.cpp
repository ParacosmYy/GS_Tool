#include "f18105/m18105.h"
QVector<double> m18105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m17712/m17712.h"
QVector<double> m17712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

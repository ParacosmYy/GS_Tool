#include "k18530/m18530.h"
QVector<double> m18530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

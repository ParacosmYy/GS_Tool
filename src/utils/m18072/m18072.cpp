#include "m18072/m18072.h"
QVector<double> m18072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

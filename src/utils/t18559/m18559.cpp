#include "t18559/m18559.h"
QVector<double> m18559::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "h8887/m8887.h"
QVector<double> m8887::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

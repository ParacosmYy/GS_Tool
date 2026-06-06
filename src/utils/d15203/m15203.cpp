#include "d15203/m15203.h"
QVector<double> m15203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

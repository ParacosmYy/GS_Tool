#include "f18825/m18825.h"
QVector<double> m18825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

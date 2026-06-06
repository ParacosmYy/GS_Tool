#include "g24926/m24926.h"
QVector<double> m24926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

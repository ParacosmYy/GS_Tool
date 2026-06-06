#include "f18585/m18585.h"
QVector<double> m18585::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "d24003/m24003.h"
QVector<double> m24003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

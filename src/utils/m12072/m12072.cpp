#include "m12072/m12072.h"
QVector<double> m12072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "g24806/m24806.h"
QVector<double> m24806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

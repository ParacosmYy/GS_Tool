#include "g24306/m24306.h"
QVector<double> m24306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "g9306/m9306.h"
QVector<double> m9306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

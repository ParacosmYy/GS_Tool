#include "g24406/m24406.h"
QVector<double> m24406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

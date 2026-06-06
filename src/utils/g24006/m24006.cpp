#include "g24006/m24006.h"
QVector<double> m24006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

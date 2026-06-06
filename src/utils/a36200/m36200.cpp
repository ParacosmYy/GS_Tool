#include "a36200/m36200.h"
QVector<double> m36200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

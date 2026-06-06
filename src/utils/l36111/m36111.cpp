#include "l36111/m36111.h"
QVector<double> m36111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

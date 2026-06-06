#include "l36011/m36011.h"
QVector<double> m36011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

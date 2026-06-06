#include "l36031/m36031.h"
QVector<double> m36031::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

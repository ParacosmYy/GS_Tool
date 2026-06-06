#include "l36211/m36211.h"
QVector<double> m36211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

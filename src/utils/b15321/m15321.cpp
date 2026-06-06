#include "b15321/m15321.h"
QVector<double> m15321::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

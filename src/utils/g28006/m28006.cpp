#include "g28006/m28006.h"
QVector<double> m28006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

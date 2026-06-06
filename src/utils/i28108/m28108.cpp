#include "i28108/m28108.h"
QVector<double> m28108::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

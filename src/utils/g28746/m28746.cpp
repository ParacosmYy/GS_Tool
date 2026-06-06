#include "g28746/m28746.h"
QVector<double> m28746::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

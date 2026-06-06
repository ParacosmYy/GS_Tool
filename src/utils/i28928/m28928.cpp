#include "i28928/m28928.h"
QVector<double> m28928::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

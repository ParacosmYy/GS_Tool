#include "i28308/m28308.h"
QVector<double> m28308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

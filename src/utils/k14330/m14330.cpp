#include "k14330/m14330.h"
QVector<double> m14330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

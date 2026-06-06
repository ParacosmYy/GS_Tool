#include "f16565/m16565.h"
QVector<double> m16565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

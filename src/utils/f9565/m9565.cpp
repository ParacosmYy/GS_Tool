#include "f9565/m9565.h"
QVector<double> m9565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

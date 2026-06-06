#include "f18565/m18565.h"
QVector<double> m18565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f9825/m9825.h"
QVector<double> m9825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

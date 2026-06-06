#include "a24240/m24240.h"
QVector<double> m24240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

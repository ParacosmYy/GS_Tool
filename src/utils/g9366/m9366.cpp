#include "g9366/m9366.h"
QVector<double> m9366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

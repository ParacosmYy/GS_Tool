#include "g28366/m28366.h"
QVector<double> m28366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "h9387/m9387.h"
QVector<double> m9387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

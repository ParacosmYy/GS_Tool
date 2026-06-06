#include "g18366/m18366.h"
QVector<double> m18366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

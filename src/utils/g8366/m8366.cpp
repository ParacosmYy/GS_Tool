#include "g8366/m8366.h"
QVector<double> m8366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

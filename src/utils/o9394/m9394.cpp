#include "o9394/m9394.h"
QVector<double> m9394::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "n7873/m7873.h"
QVector<double> m7873::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

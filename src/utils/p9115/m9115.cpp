#include "p9115/m9115.h"
QVector<double> m9115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

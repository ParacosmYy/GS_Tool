#include "f9845/m9845.h"
QVector<double> m9845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

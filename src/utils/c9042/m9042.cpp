#include "c9042/m9042.h"
QVector<double> m9042::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

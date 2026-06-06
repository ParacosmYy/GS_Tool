#include "n9053/m9053.h"
QVector<double> m9053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

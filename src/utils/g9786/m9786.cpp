#include "g9786/m9786.h"
QVector<double> m9786::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

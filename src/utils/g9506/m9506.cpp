#include "g9506/m9506.h"
QVector<double> m9506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

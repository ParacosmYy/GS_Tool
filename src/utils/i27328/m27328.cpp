#include "i27328/m27328.h"
QVector<double> m27328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "b9021/m9021.h"
QVector<double> m9021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

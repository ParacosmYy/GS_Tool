#include "g9046/m9046.h"
QVector<double> m9046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

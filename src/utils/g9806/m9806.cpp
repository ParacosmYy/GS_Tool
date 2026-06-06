#include "g9806/m9806.h"
QVector<double> m9806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

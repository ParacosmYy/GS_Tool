#include "g9566/m9566.h"
QVector<double> m9566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

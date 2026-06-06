#include "b9701/m9701.h"
QVector<double> m9701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

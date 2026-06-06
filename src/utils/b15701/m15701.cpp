#include "b15701/m15701.h"
QVector<double> m15701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "g8066/m8066.h"
QVector<double> m8066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

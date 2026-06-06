#include "m36832/m36832.h"
QVector<double> m36832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

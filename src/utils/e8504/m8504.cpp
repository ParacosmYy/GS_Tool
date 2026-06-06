#include "e8504/m8504.h"
QVector<double> m8504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

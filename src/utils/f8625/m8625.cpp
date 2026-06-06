#include "f8625/m8625.h"
QVector<double> m8625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

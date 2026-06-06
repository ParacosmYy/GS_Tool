#include "n8553/m8553.h"
QVector<double> m8553::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

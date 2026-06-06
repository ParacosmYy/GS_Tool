#include "f8385/m8385.h"
QVector<double> m8385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

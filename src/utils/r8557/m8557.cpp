#include "r8557/m8557.h"
QVector<double> m8557::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

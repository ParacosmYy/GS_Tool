#include "r8257/m8257.h"
QVector<double> m8257::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "r8737/m8737.h"
QVector<double> m8737::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "s8078/m8078.h"
QVector<double> m8078::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

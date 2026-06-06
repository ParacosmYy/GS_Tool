#include "s9858/m9858.h"
QVector<double> m9858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

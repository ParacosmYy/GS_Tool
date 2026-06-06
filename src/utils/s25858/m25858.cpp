#include "s25858/m25858.h"
QVector<double> m25858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

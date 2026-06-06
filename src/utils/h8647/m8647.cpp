#include "h8647/m8647.h"
QVector<double> m8647::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

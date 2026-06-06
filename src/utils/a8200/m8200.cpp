#include "a8200/m8200.h"
QVector<double> m8200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

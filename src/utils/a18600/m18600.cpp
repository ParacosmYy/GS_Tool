#include "a18600/m18600.h"
QVector<double> m18600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

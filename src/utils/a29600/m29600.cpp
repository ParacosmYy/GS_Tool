#include "a29600/m29600.h"
QVector<double> m29600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

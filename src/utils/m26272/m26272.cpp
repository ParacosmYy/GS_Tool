#include "m26272/m26272.h"
QVector<double> m26272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "a19600/m19600.h"
QVector<double> m19600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

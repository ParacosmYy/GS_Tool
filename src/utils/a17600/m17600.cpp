#include "a17600/m17600.h"
QVector<double> m17600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

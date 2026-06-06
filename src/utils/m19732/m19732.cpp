#include "m19732/m19732.h"
QVector<double> m19732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

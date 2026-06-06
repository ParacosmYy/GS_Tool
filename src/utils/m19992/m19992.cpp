#include "m19992/m19992.h"
QVector<double> m19992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

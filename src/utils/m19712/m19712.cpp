#include "m19712/m19712.h"
QVector<double> m19712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

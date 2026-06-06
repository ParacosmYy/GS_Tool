#include "m19152/m19152.h"
QVector<double> m19152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

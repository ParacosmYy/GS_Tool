#include "m26112/m26112.h"
QVector<double> m26112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

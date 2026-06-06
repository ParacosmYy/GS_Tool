#include "s15258/m15258.h"
QVector<double> m15258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

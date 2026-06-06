#include "e29004/m29004.h"
QVector<double> m29004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

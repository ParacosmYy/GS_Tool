#include "a30800/m30800.h"
QVector<double> m30800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

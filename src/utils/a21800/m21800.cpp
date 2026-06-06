#include "a21800/m21800.h"
QVector<double> m21800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

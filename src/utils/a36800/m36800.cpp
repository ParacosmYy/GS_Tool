#include "a36800/m36800.h"
QVector<double> m36800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

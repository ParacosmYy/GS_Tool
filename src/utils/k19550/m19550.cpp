#include "k19550/m19550.h"
QVector<double> m19550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

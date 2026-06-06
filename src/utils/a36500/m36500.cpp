#include "a36500/m36500.h"
QVector<double> m36500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

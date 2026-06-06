#include "a36920/m36920.h"
QVector<double> m36920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

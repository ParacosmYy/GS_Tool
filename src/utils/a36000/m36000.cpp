#include "a36000/m36000.h"
QVector<double> m36000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f36825/m36825.h"
QVector<double> m36825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

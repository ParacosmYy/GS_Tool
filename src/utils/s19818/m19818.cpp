#include "s19818/m19818.h"
QVector<double> m19818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

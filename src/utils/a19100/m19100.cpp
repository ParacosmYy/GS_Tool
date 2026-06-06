#include "a19100/m19100.h"
QVector<double> m19100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

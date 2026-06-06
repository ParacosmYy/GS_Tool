#include "a19500/m19500.h"
QVector<double> m19500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

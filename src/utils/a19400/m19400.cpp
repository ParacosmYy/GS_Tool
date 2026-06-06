#include "a19400/m19400.h"
QVector<double> m19400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

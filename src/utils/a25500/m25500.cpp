#include "a25500/m25500.h"
QVector<double> m25500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

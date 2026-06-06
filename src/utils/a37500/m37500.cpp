#include "a37500/m37500.h"
QVector<double> m37500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

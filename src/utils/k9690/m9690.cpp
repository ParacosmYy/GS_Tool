#include "k9690/m9690.h"
QVector<double> m9690::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

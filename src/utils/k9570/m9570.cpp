#include "k9570/m9570.h"
QVector<double> m9570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

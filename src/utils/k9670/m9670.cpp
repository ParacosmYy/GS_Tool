#include "k9670/m9670.h"
QVector<double> m9670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

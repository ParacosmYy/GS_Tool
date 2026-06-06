#include "s12878/m12878.h"
QVector<double> m12878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

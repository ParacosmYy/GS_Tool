#include "h9767/m9767.h"
QVector<double> m9767::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

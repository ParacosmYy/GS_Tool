#include "s9038/m9038.h"
QVector<double> m9038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "f29005/m29005.h"
QVector<double> m29005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

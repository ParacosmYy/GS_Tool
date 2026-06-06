#include "e16824/m16824.h"
QVector<double> m16824::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

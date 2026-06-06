#include "e32824/m32824.h"
QVector<double> m32824::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

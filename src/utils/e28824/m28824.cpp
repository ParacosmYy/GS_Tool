#include "e28824/m28824.h"
QVector<double> m28824::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

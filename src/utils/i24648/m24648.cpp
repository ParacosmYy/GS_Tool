#include "i24648/m24648.h"
QVector<double> m24648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

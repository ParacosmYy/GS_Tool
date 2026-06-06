#include "m34412/m34412.h"
QVector<double> m34412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

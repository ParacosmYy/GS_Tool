#include "i12008/m12008.h"
QVector<double> m12008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

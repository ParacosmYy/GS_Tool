#include "i25648/m25648.h"
QVector<double> m25648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

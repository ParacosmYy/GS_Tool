#include "i28648/m28648.h"
QVector<double> m28648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

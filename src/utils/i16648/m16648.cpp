#include "i16648/m16648.h"
QVector<double> m16648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "i9648/m9648.h"
QVector<double> m9648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

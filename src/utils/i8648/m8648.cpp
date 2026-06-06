#include "i8648/m8648.h"
QVector<double> m8648::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

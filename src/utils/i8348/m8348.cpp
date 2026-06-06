#include "i8348/m8348.h"
QVector<double> m8348::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

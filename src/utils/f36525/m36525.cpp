#include "f36525/m36525.h"
QVector<double> m36525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m34632/m34632.h"
QVector<double> m34632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

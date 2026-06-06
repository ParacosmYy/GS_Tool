#include "e34024/m34024.h"
QVector<double> m34024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

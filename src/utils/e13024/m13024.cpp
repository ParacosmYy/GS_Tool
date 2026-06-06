#include "e13024/m13024.h"
QVector<double> m13024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

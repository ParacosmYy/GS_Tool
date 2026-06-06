#include "e21024/m21024.h"
QVector<double> m21024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

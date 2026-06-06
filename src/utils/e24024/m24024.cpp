#include "e24024/m24024.h"
QVector<double> m24024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

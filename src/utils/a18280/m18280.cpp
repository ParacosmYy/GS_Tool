#include "a18280/m18280.h"
QVector<double> m18280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

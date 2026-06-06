#include "q32016/m32016.h"
QVector<double> m32016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

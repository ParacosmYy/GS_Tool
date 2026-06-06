#include "b18221/m18221.h"
QVector<double> m18221::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "s7858/m7858.h"
QVector<double> m7858::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

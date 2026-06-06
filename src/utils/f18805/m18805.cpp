#include "f18805/m18805.h"
QVector<double> m18805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

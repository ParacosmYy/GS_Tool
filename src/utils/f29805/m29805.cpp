#include "f29805/m29805.h"
QVector<double> m29805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

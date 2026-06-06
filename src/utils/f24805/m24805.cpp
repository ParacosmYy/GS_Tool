#include "f24805/m24805.h"
QVector<double> m24805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

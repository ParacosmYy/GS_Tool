#include "b18241/m18241.h"
QVector<double> m18241::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

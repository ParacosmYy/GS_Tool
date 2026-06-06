#include "i27808/m27808.h"
QVector<double> m27808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

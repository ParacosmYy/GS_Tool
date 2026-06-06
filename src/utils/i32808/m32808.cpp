#include "i32808/m32808.h"
QVector<double> m32808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
